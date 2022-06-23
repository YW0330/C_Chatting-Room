#include <winsock2.h> // 使用Winsock API函數
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#define MAX_CLNT 256
#define BUF_SIZE 100
// 告訴編譯器與WS2_32庫連接
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma comment(lib, "ws2_32.lib")

void error_handling(const char *msg);    /*錯誤處理函數*/
DWORD WINAPI ThreadProc(LPVOID lpParam); /*執行緒執行函數*/
void send_msg(char *msg, int len);       /*發送訊息函數*/
HANDLE g_hEvent;                         /*事件對象*/
int clnt_cnt = 0;                        //統計socket
int clnt_socks[MAX_CLNT];                //管理socket
HANDLE hThread[MAX_CLNT];                //管理執行緒

int main()
{
    // 1.請求協議版本
    WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
    WSAStartup(sockVersion, &wsaData); // 請求一個2.2版本的socket

    // 創建一個自動重置的（auto-reset events），有訊號的（signaled）事件
    g_hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

    // 2.創建socket
    SOCKET serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock == INVALID_SOCKET)
        error_handling("Failed socket()");

    // 3.創建協議地址
    SOCKADDR_IN sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8888);            // 8888端口 0-65535
    sin.sin_addr.S_un.S_addr = INADDR_ANY; //本地地址
    // sin.sin_addr.S_un.S_addr = inet_addr("192.168.1.103");

    // 4.绑定
    if (bind(serv_sock, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR)
        error_handling("Failed bind()");

    // 5.監聽
    if (listen(serv_sock, 10) == SOCKET_ERROR)
        error_handling("Failed listen()");
    printf("Start listen:\n");

    //  6.等待客戶端連接
    SOCKADDR_IN remoteAddr;
    int nAddrLen = sizeof(remoteAddr);
    DWORD dwThreadId; /*執行緒ID*/
    SOCKET clnt_sock;

    while (TRUE)
    {
        printf("等待新連接\n");
        // 接受一個新連接
        clnt_sock = accept(serv_sock, (SOCKADDR *)&remoteAddr, &nAddrLen);
        if (clnt_sock == INVALID_SOCKET)
        {
            printf("Failed accept()");
            continue;
        }
        /*等待事件對象狀態信號*/
        WaitForSingleObject(g_hEvent, INFINITE);
        hThread[clnt_cnt] = CreateThread(
            NULL,               // 默認安全属性
            0,                  // 默認堆疊大小
            ThreadProc,         // 執行緒入口地址（執行執行緒的函數）
            (void *)&clnt_sock, // 傳給函數的參數
            0,                  // 指定執行緒立即運行
            &dwThreadId);       // 返回執行緒的ID
        clnt_socks[clnt_cnt++] = clnt_sock;
        SetEvent(g_hEvent); /*設置信號*/

        printf(" 接收到一個連接：%s 執行緒ID：%lu\r\n", inet_ntoa(remoteAddr.sin_addr), dwThreadId);
    }
    WaitForMultipleObjects(clnt_cnt, hThread, true, INFINITE);
    for (int i = 0; i < clnt_cnt; i++)
    {
        CloseHandle(hThread[i]);
    }
    // 關閉 Socket
    closesocket(serv_sock);
    // 釋放 WS2_32 庫
    WSACleanup();
    return 0;
}

void error_handling(const char *msg)
{
    printf("%s\n", msg);
    WSACleanup();
    exit(1);
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    int clnt_sock = *((int *)lpParam);
    int str_len = 0, i;
    char msg[BUF_SIZE];

    while ((str_len = recv(clnt_sock, msg, sizeof(msg), 0)) != -1)
    {
        send_msg(msg, str_len);
        printf("群組發送成功\n");
    }
    printf("客戶端退出:%lu\n", GetCurrentThreadId());
    /*等待事件對象狀態信號*/
    WaitForSingleObject(g_hEvent, INFINITE);
    for (i = 0; i < clnt_cnt; i++)
    {
        if (clnt_sock == clnt_socks[i])
        {
            while (i++ < clnt_cnt - 1)
                clnt_socks[i] = clnt_socks[i + 1];
            break;
        }
    }
    clnt_cnt--;
    SetEvent(g_hEvent); /*設置信號*/
    // 關閉客戶端連接
    closesocket(clnt_sock);
    return 0;
}

void send_msg(char *msg, int len)
{
    int i;
    /*等待事件對象狀態信號*/
    WaitForSingleObject(g_hEvent, INFINITE);
    for (i = 0; i < clnt_cnt; i++)
        send(clnt_socks[i], msg, len, 0);
    SetEvent(g_hEvent); /*設置信號*/
}