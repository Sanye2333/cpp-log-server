# 高并发日志服务端（C++）
基于 C++、Windows Socket、多线程实现的日志服务器，支持 TCP 粘包处理、UTF-8 中文、日志持久化。

## 功能特点
1. 多线程处理客户端连接，高并发支持；
2. 解决 TCP 粘包问题，精准解析日志行；
3. UTF-8 中文编码适配，控制台/文件无乱码；
4. 日志按日期自动生成文件，异步写文件提升性能；
5. 支持配置文件加载，端口/缓冲区大小可配置。

## 编译运行
### 编译
```bash
g++ src/server.cpp src/tcpSocket.c src/config.cpp -o build/server.exe -lws2_32 -lpthread -mconsole
