#include <stdio.h>
#include <windows.h>

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
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == SOCKET_ERROR)
    {
        printf("創建socket失敗!\n");
        WSACleanup(); // 清理協議版本
        return -2;
    }
    printf("創建socket成功!\n");

    // 3.創建協議地址族
    SOCKADDR_IN addr = {0};
    addr.sin_family = AF_INET;                              // 協議版本
    addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.113"); // IP位置
    addr.sin_port = htons(10086);                           // 0-65535
    // 4.綁定
    int r = bind(serverSocket, (SOCKADDR *)&addr, sizeof addr);
    if (r == -1)
    {
        printf("bind失敗");
        closesocket(serverSocket);
        WSACleanup();
        return -2;
    }
    printf("bind成功");

    // 5.監聽
    r = listen(serverSocket, 10);
    if (r == -1)
    {
        printf("listen失敗");
        closesocket(serverSocket);
        WSACleanup();
        return -2;
    }
    printf("listen成功\n");

    // 6.等待客戶端連接
    // 客戶端協議地址族
    SOCKADDR_IN cAddr = {0};
    int len = sizeof cAddr;
    SOCKET clientSocket = accept(serverSocket, (SOCKADDR *)&cAddr, &len);
    if (clientSocket == SOCKET_ERROR)
    {
        printf("服務器當機了!\n");
        closesocket(serverSocket);
        WSACleanup();
        return -2;
    }
    printf("有客戶端連接到服務器了:%s!\n", inet_ntoa(cAddr.sin_addr));
    // 7. 通信
    char buff[1024];
    while (1)
    {
        r = recv(clientSocket, buff, 1023, 0);
        if (r > 0)
        {
            buff[r] = 0; //添加'\0'
            printf(">>%s\n", buff);
        }
    }
    return 0;
}