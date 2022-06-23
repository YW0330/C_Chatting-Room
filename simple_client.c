#include <stdio.h>
#include <windows.h>
#include <pthread.h>

int main()
{
    // 1.請求協議版本
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        printf("請求協議版本失敗!\n");
        return -1;
    }
    printf("請求協議成功!\n");

    // 2.創建socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == SOCKET_ERROR)
    {
        printf("創建socket失敗!\n");
        WSACleanup(); // 清理協議版本
        return -2;
    }
    printf("創建socket成功!\n");

    // 3.獲取協議地址族
    SOCKADDR_IN addr = {0};
    addr.sin_family = AF_INET;                              // 協議版本
    addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.113"); // IP位置
    addr.sin_port = htons(10086);                           // 0-65535

    // 4.連接服務器
    int r = connect(clientSocket, (SOCKADDR *)&addr, sizeof addr);
    if (r == -1)
    {
        printf("連接服務器失敗!\n");
        return -1;
    }
    printf("連接服務器成功!\n");

    // 5.通信
    char buff[1024];
    while (1)
    {
        memset(buff, 0, 1024);
        printf("你想說啥:");
        scanf("%s", buff);
        r = send(clientSocket, buff, strlen(buff), 0);
    }
    return 0;
}