#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

// 編譯器與 WS2_32 庫連接
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma comment(lib, "ws2_32.lib")

// 訊息最大長度
#define BUF_SIZE 256
// 暱稱最大長度
#define NAME_SIZE 30

DWORD WINAPI send_msg(LPVOID lpParam);
DWORD WINAPI recv_msg(LPVOID lpParam);
void error_handling(const char *msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main()
{
    HANDLE hThread[2];
    DWORD dwThreadId;
    // 初始化 WS2_32.dll
    WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
    WSAStartup(sockVersion, &wsaData);

    /*設置使用者名稱*/
    printf("Input your Chat Name:");
    scanf("%s", name);
    getchar(); //接收换行符
    // 創建 Socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
        error_handling("Failed socket()");

    // 填寫遠端伺服器訊息
    SOCKADDR_IN servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(8888);
    // 使用本地地址127.0.0.1
    servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    if (connect(sock, (SOCKADDR *)&servAddr, sizeof(servAddr)) == -1)
        error_handling("Failed connect()");
    printf("connect success\n");
    hThread[0] = CreateThread(
        NULL,         // 默認安全属性
        0,            // 默認堆疊大小
        send_msg,     // 執行緒入口地址（執行執行緒的函數）
        &sock,        // 傳給函數的參數
        0,            // 指定執行緒立即運行
        &dwThreadId); // 返回執行緒的ID
    hThread[1] = CreateThread(
        NULL,         // 默認安全属性
        0,            // 默認堆疊大小
        recv_msg,     // 執行緒入口地址（執行執行緒的函數）
        &sock,        // 傳給函數的參數
        0,            // 指定執行緒立即運行
        &dwThreadId); // 返回執行緒的ID

    // 等待執行緒結束
    WaitForMultipleObjects(2, hThread, true, INFINITE);
    CloseHandle(hThread[0]);
    CloseHandle(hThread[1]);
    printf(" Thread Over,Enter anything to over.\n");
    getchar();
    // 關閉 Socket
    closesocket(sock);
    // 釋放 WS2_32 庫
    WSACleanup();
    return 0;
}

DWORD WINAPI send_msg(LPVOID lpParam)
{
    int sock = *((int *)lpParam);
    char name_msg[NAME_SIZE + BUF_SIZE];
    while (1)
    {
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            closesocket(sock);
            exit(0);
        }
        sprintf(name_msg, "[%s]: %s", name, msg);
        int nRecv = send(sock, name_msg, strlen(name_msg), 0);
    }
    return 0;
}

DWORD WINAPI recv_msg(LPVOID lpParam)
{
    int sock = *((int *)lpParam);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;
    while (1)
    {
        str_len = recv(sock, name_msg, NAME_SIZE + BUF_SIZE - 1, 0);
        if (str_len == -1)
            return -1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return 0;
}

void error_handling(const char *msg)
{
    printf("%s\n", msg);
    WSACleanup();
    exit(1);
}