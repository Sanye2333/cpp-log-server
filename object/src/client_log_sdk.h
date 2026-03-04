#ifndef CLIENT_LOG_SDK_H
#define CLIENT_LOG_SDK_H

#include "tcpSocket.h"
// 初始化SDK 连接日志服务端
int log_sdk_init(const char *server_ip, int port);

// 上报INFO等各个级别日志
void log_info(const char *module, const char *content);
void log_error(const char *module, const char *content);

// 销毁连接
void log_sdk_destory();

#endif
