#include "client_log_sdk.h"
#include <time.h>
#include <string.h>
#include <windows.h>
// 所有SDK均使用这一个
static SOCKET g_client_fd = INVALID_SOCKET;
static void get_timestamp(char *buf, int buf_len)
{
    time_t now = time(NULL);
    // 转换为时间戳格式存入buf
    strftime(buf, buf_len, "%Y-%m-%d %H:%M-%S", localtime(&now));
}
// 序列化日志并且发送
static void log_send(LogLevel level, const char *module, const char *content)
{
    // 防止未连接服务端就发送日志
    if (g_client_fd == INVALID_SOCKET)
    {
        err("SDK未初始化,日志发送失败!");
        return;
    }

    // 时间戳
    char timestamp[32] = {0};
    get_timestamp(timestamp, sizeof(timestamp));

    // 序列化日志
    char send_buf[1024] = {0};
    snprintf(send_buf, sizeof(send_buf),
             "%s|%d|%s|%s\n",
             timestamp,
             level,
             module,
             content);

    // 调用底层SOCKET发送日志
    send(g_client_fd, send_buf, strlen(send_buf), 0);
}
//-----------对外接口----------
// 初始化SDK 连接日志服务端
int log_sdk_init(const char *server_ip, int port)
{
    SetConsoleOutputCP(936);
    SetConsoleCP(936);
    // 初始化Socket库
    if (!init_Socket())
    {
        return -1;
    }

    // 创建客户端并连接服务端
    g_client_fd = createClientSocket(server_ip);
    return (g_client_fd == INVALID_SOCKET) ? -1 : 0;
}

// 上报各级别日志
void log_info(const char *module, const char *content)
{
    log_send(LOG_INFO, module, content);
}
void log_error(const char *module, const char *content)
{
    log_send(LOG_ERROR, module, content);
}

// 销毁SDK
void log_sdk_destory()
{
    // 关闭Socket
    if (g_client_fd != INVALID_SOCKET)
    {
        closesocket(g_client_fd);
    }

    // 释放Socket库资源
    close_Socket();
}
