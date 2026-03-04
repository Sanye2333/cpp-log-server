#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <stdbool.h>
#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define PORT 11451
#define err(errMsg) printf("[Line:%d]%s failed code %d", __LINE__, errMsg, WSAGetLastError());
// UTF-8转GBK（解决中文乱码核心函数）
char *UTF8ToGBK(const char *utf8_str);
// 打开库
bool init_Socket();

// 关闭库
bool close_Socket();

// 创建服务器socket
SOCKET createServerSocket();
SOCKET createServerSocket(const char *ip, int port);

// 创建客户端socket
SOCKET createClientSocket(const char *ip);

// 日志结构
typedef enum
{ // 枚举
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

typedef struct
{
    char timestamp[32]; // 时间戳 "2026-03-04 16:00:00"
    LogLevel level;     // 日志级别
    char module[32];    // 模块名
    char content[512];  // 日志内容
} LogData;

#endif