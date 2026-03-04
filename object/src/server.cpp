#include "tcpSocket.h"
#include "config.h"
// g++ src/server.cpp src/tcpSocket.c src/config.cpp -o build/server.exe -lws2_32 -lpthread -mconsole

#include <pthread.h>
#include <queue>
#include <mutex>
#include <fstream>
#include <ctime>
#include <string>
#include <cstdlib>
#include <windows.h>

std::queue<LogData> g_log_queue;
std::mutex g_log_mutex;

// 文件锁&线程退出标记
bool g_exit_write_thread = false;
std::mutex g_file_mutex;

// 全局配置
ServerConfig g_cfg;
HANDLE g_hConsole = NULL;

void print_wchar(const wchar_t *wstr)
{
    if (g_hConsole == NULL)
        return;
    DWORD written;
    WriteConsoleW(g_hConsole, wstr, wcslen(wstr), &written, NULL);
}

// UTF-8转宽字符防止乱码
wchar_t *utf8_to_wchar(const char *utf8_str)
{
    // 使用Windows原生API替代废弃的wstring_convert
    int len = MultiByteToWideChar(65001, 0, utf8_str, -1, NULL, 0);
    wchar_t *res = new wchar_t[len];
    MultiByteToWideChar(65001, 0, utf8_str, -1, res, len);
    return res;
}

// 线程函数
void *handle_client(void *arg)
{
    SOCKET clifd = *(SOCKET *)arg;
    free(arg);
    char recv_buf[g_cfg.recv_buf_size] = {0};
    char leftover[2048] = {0};
    while (1)
    {
        // 循环接收日志
        int recv_len = recv(clifd, recv_buf, sizeof(recv_buf) - 1, 0);
        if (recv_len <= 0)
        {
            print_wchar(L"客户端断开连接/接收失败,fd=");
            printf("%d\n", clifd);

            closesocket(clifd);
            pthread_exit(NULL);
        }

        recv_buf[recv_len] = '\0';
        int leftover_len = strlen(leftover);
        if (leftover_len + recv_len < sizeof(leftover) - 1) // 预留终止符空间
        {
            strcat(leftover, recv_buf);
        }
        else
        {
            print_wchar(L"缓冲区满清空\n");
            memset(leftover, 0, sizeof(leftover));
            strncat(leftover, recv_buf, sizeof(leftover) - 1);
        }

        char *pos = NULL;
        char temp_buf[2048] = {0};
        strcpy(temp_buf, leftover);
        memset(leftover, 0, sizeof(leftover));

        while ((pos = strchr(temp_buf, '\n')) != NULL)
        {
            *pos = '\0';
            char line[1024] = {0};
            strncpy(line, temp_buf, sizeof(line) - 1);

            LogData log = {0};
            char *saveptr = NULL;
            char *token = strtok_r(line, "|", &saveptr);
            if (token)
                strncpy(log.timestamp, token, sizeof(log.timestamp) - 1);

            token = strtok_r(NULL, "|", &saveptr);
            if (token)
                log.level = (LogLevel)atoi(token);

            token = strtok_r(NULL, "|", &saveptr);
            if (token)
                strncpy(log.module, token, sizeof(log.module) - 1);

            token = strtok_r(NULL, "|", &saveptr);
            if (token)
                strncpy(log.content, token, sizeof(log.content) - 1);

            // 加锁入队
            g_log_mutex.lock();
            g_log_queue.push(log);
            g_log_mutex.unlock();

            print_wchar(L"收到日志：[");
            printf("%s][%s][", log.timestamp, log.level == LOG_INFO ? "INFO" : "ERROR");
            wchar_t *w_module = utf8_to_wchar(log.module);
            print_wchar(w_module);
            print_wchar(L"] ");
            wchar_t *w_content = utf8_to_wchar(log.content);
            print_wchar(w_content);
            print_wchar(L"\n");
            delete[] w_module;
            delete[] w_content;

            strcpy(temp_buf, pos + 1);
        }

        if (strlen(temp_buf) > 0)
        {
            strncpy(leftover, temp_buf, sizeof(leftover) - 1);
        }
    }
}

// 生成日志文件名
std::string get_log_filename()
{
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_s(&tm_now, &now);

    char filename[128] = {0};
    snprintf(filename, sizeof(filename), "%s%04d-%02d-%02d.log",
             g_cfg.log_dir.c_str(),
             tm_now.tm_year + 1900,
             tm_now.tm_mon + 1,
             tm_now.tm_mday);
    return std::string(filename);
}

void *write_log_to_file(void *arg)
{
    while (!g_exit_write_thread)
    {
        LogData log = {0};
        bool has_log = false;

        g_log_mutex.lock();
        if (!g_log_queue.empty())
        {
            log = g_log_queue.front();
            g_log_queue.pop();
            has_log = true;
        }
        g_log_mutex.unlock();

        if (has_log)
        {
            g_file_mutex.lock();
            std::string filename = get_log_filename();
            std::ofstream log_file(filename, std::ios::app | std::ios::out | std::ios::binary);
            if (!log_file.is_open())
            {
                print_wchar(L"打开日志文件失败：");
                printf("%s\n", filename.c_str());
                g_file_mutex.unlock();
                continue;
            }

            // 格式化
            char log_line[1024] = {0};
            snprintf(log_line, sizeof(log_line) - 1,
                     "[%s][%s][%s] %s\n",
                     log.timestamp,
                     log.level == LOG_INFO ? "INFO" : "ERROR",
                     log.module,
                     log.content);
            log_file.write(log_line, strlen(log_line));
            log_file.flush();
            log_file.close();
            g_file_mutex.unlock();
        }
        else
        {
            Sleep(10); // 目的为减少CPU占用
        }
    }

    g_log_mutex.lock();
    while (!g_log_queue.empty())
    {
        LogData log = g_log_queue.front();
        g_log_queue.pop();

        g_file_mutex.lock();
        std::string filename = get_log_filename();
        std::ofstream log_file(filename, std::ios::app | std::ios::out | std::ios::binary);
        if (log_file.is_open())
        {
            char log_line[1024] = {0};
            snprintf(log_line, sizeof(log_line) - 1,
                     "[%s][%s][%s] %s\n",
                     log.timestamp,
                     log.level == LOG_INFO ? "INFO" : "ERROR",
                     log.module,
                     log.content);
            log_file.write(log_line, strlen(log_line));
            log_file.flush();
            log_file.close();
        }
        g_file_mutex.unlock();
    }
    g_log_mutex.unlock();

    pthread_exit(NULL);
    return NULL;
}

int main()
{
    // 初始化控制台输出句柄
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_hConsole == INVALID_HANDLE_VALUE)
    {
        printf("CAN NOT GET HANDLE!\n");
        return -1;
    }

    g_cfg = load_config("./log_server.conf");

    // 避免非法值
    if (g_cfg.server_port < 1 || g_cfg.server_port > 65535)
    {
        print_wchar(L"配置文件端口非法(");
        printf("%d", g_cfg.server_port);
        print_wchar(L"),使用默认端口11451\n");
        g_cfg.server_port = 11451;
    }
    if (g_cfg.recv_buf_size < 1024 || g_cfg.recv_buf_size > 8192)
    {
        print_wchar(L"配置文件缓冲区大小非法(");
        printf("%d", g_cfg.recv_buf_size);
        print_wchar(L"),使用默认值4096\n");
        g_cfg.recv_buf_size = 4096;
    }

    // 创建日志目录
#ifdef _WIN32
    CreateDirectoryA(g_cfg.log_dir.c_str(), NULL);
#else
    mkdir(g_cfg.log_dir.c_str(), 0755);
#endif

    // 初始化Socket库
    init_Socket();

    pthread_t write_tid;
    int ret = pthread_create(&write_tid, NULL, write_log_to_file, NULL);
    if (ret != 0)
    {
        print_wchar(L"启动写文件线程失败!\n");
        return -1;
    }
    pthread_detach(write_tid);

    // 创建服务端Socket
    SOCKET serfd = createServerSocket(g_cfg.server_ip.c_str(), g_cfg.server_port);
    if (serfd == INVALID_SOCKET)
    {
        print_wchar(L"创建服务端Socket失败!\n");
        return -1;
    }
    print_wchar(L"服务端启动成功\n");

    while (1)
    {
        SOCKET clifd = accept(serfd, NULL, NULL);
        if (clifd == INVALID_SOCKET)
        {
            err("accept");
            continue;
        }
        print_wchar(L"新客户端连接,fd=");
        printf("%d\n", clifd);

        pthread_t tid;
        SOCKET *p_clifd = (SOCKET *)malloc(sizeof(SOCKET));
        *p_clifd = clifd;
        ret = pthread_create(&tid, NULL, handle_client, p_clifd);
        if (ret != 0)
        {
            print_wchar(L"创建线程失败,fd=");
            printf("%d\n", clifd);
            closesocket(clifd);
            free(p_clifd);
            continue;
        }

        pthread_detach(tid);
    }
    g_exit_write_thread = true;
    closesocket(serfd);
    close_Socket();
    return 0;
}