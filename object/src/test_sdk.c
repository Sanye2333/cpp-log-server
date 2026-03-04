#include "client_log_sdk.h"
// g++ src/test_sdk.c src/client_log_sdk.c src/tcpSocket.c -o build/test_sdk.exe -lws2_32 -lpthread -mconsole
// g++ src/server.cpp src/tcpSocket.c -o build/server.exe -lws2_32 -lpthread
//如果客户端是 GBK 编码 → 要么改客户端为 UTF-8，要么把服务端 utf8_to_wchar 里的 65001 改成 936（GBK 代码页）。
// 客户端发送日志时，确保 2 点：
// ① 字符串是 UTF-8 编码（C++ 里直接写中文，编译时加 -fexec-charset=utf-8 即可）
// ② 格式严格为：时间戳|级别|模块|内容 + 换行符\n
//std::string log = "2026-03-05 10:00:00|1|用户模块|用户张三登录成功\n";
//send(sock, log.c_str(), log.length(), 0); // 直接发送，无需额外处理
int main()
{
    // 1. 初始化SDK（连接127.0.0.1:11451的服务端）
    if (log_sdk_init("127.0.0.1", 11451) != 0)
    {
        printf("SDK初始化失败！\n");
        return -1;
    }

    // 2. 上报日志（一行代码搞定，不用关心底层）
    log_info("用户模块", "用户张三登录成功");
    log_error("订单模块", "订单123支付超时");

    sleep(100);
    // 3. 销毁SDK
    log_sdk_destory();
    getchar(); // 防止闪退
    return 0;
}