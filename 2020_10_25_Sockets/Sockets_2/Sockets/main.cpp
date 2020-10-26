//Code by:
//- Jaume Montagut
//Thanks to:
//- Jesus Diaz
//- https://www.binarytides.com/udp-socket-programming-in-winsock/

//Client
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"
#include <stdio.h>
#include <stdlib.h>

#define LOCAL_PORT 8888
#define BUF_LEN 1024

int main(int argc, char** argv)
{
    //INFO: Initialization
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
    {
        printf("Error: %d (See error codes in https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2)", WSAGetLastError());
        return 1;
    }

    //INFO: Socket creation
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        printf("Error: %d (See error codes in https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2)", WSAGetLastError());
        return 1;
    }
        
    //INFO: Connect the socket
    struct sockaddr_in dstAddr;
    int dstAddrSize = sizeof(dstAddr);
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port = htons(LOCAL_PORT);
    inet_pton(AF_INET, "127.0.0.1", &dstAddr.sin_addr);

    //INFO: Send information
    char sendBuf[BUF_LEN] = "ping\0";
    char recvBuf[BUF_LEN];

    printf("Waiting to send message to server...\n");

    for (int i = 0; i < 5; ++i) {
        if (sendto(s, sendBuf, BUF_LEN, 0, (SOCKADDR*)&dstAddr, dstAddrSize) == SOCKET_ERROR) {
            printf("Error: %d (See error codes in https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2)", WSAGetLastError());
            return 1;
        }
        memset(recvBuf, '/0', BUF_LEN);
        if (recvfrom(s, recvBuf, BUF_LEN, 0, (SOCKADDR*)&dstAddr, &dstAddrSize) == SOCKET_ERROR) {
            printf("Error: %d (See error codes in https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2)", WSAGetLastError());
            return 1;
        }
        printf("%s\n", recvBuf);
    }

    //INFO: Clean up
    closesocket(s);
    WSACleanup();
    system("pause");

    return 0;
}