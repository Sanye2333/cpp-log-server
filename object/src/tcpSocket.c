#include "tcpSocket.h"
bool init_Socket()
{
    WSADATA wsadata;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
    {
        printf("WSAStartup failed code %d", WSAGetLastError());
        return false;
    }
    return true;
}

bool close_Socket()
{
    if (0 != WSACleanup())
    {
        printf("WSACleanup failed code %d", WSAGetLastError());
        return false;
    }
    return true;
}

SOCKET createServerSocket()
{
    // 新Socket
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        err("socket");
        return 0;
    }

    // 绑定IP地址&端口号
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;   // 必须如上
    addr.sin_port = htons(PORT); // 网络和计算机本地大端小端方式不同需要转换
    addr.sin_addr.S_un.S_addr = ADDR_ANY;
    if (SOCKET_ERROR == bind(fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        err("bind");
        return 0;
    }

    // 监听
    listen(fd, 10);
    return fd;
}

SOCKET createServerSocket(const char *ip, int port)
{
    SOCKET serfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serfd == INVALID_SOCKET)
    {
        err("socket创建失败");
        return INVALID_SOCKET;
    }

    // 端口复用
    int opt = 1;
    setsockopt(serfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    struct sockaddr_in ser_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    // 指定IP
    ser_addr.sin_addr.s_addr = inet_addr(ip);

    // 绑定IP和端口
    if (bind(serfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) == SOCKET_ERROR)
    {
        err("bind失败");
        closesocket(serfd);
        return INVALID_SOCKET;
    }

    // 监听,这里设置的最大监听为10，可以修改。
    if (listen(serfd, 10) == SOCKET_ERROR)
    {
        err("listen失败");
        closesocket(serfd);
        return INVALID_SOCKET;
    }

    return serfd;
}

SOCKET createClientSocket(const char *ip)
{
    // 新Socket
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        err("socket");
        return 0;
    }

    // 建立与服务器连接
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;   // 必须如上
    addr.sin_port = htons(PORT); // 网络和计算机本地大端小端方式不同需要转换
    addr.sin_addr.S_un.S_addr = inet_addr(ip);
    if (INVALID_SOCKET == connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        err("connect");
        return false;
    }
    return fd;
}
char *UTF8ToGBK(const char *utf8_str)
{
    if (utf8_str == NULL)
        return NULL;

    // 1. 计算需要的GBK缓冲区大小
    int gbk_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    wchar_t *wstr = (wchar_t *)malloc(gbk_len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, wstr, gbk_len);

    // 2. 转换为GBK编码
    int len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *gbk_str = (char *)malloc(len);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, gbk_str, len, NULL, NULL);

    free(wstr);
    return gbk_str;
}
