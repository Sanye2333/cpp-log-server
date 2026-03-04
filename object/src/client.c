#include "tcpSocket.h"
// g++ src/client.c src/tcpSocket.c -o build/client.exe -lws2_32 -lpthread

int main()
{
    init_Socket();

    SOCKET fd = createClientSocket("127.0.0.1");

    char sendbuf[BUFSIZ] = {0};
    char recvbuf[BUFSIZ] = {0};


    while (true)
    {

        // 发送消息
        printf("send>");
        fgets(sendbuf, BUFSIZ, stdin);
        // 去掉 fgets 读取的换行符（因为按回车会把 \n 也读进来）
        sendbuf[strcspn(sendbuf, "\n")] = '\0';
        if (SOCKET_ERROR == send(fd, sendbuf, strlen(sendbuf), 0))
        {
            err("send");
        }
        // 接收
        if (0 <= recv(fd, recvbuf, BUFSIZ, 0))
        {
            printf("recv:%s\n", recvbuf);
            memset(recvbuf, 0, sizeof(recvbuf));
        }
    }

    close_Socket();
    printf("client_____END_____");
    getchar();
    return 0;
}