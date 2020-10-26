//Server
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
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        printf("Error: %d", WSAGetLastError());
        return false;
    }
    
    //INFO: Socket creation
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        printf("Error: %d", WSAGetLastError());
        return false;
    }

    //Bind our adress (to listen to incoming connections)
    struct sockaddr_in srcAddr;
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_port = htons(LOCAL_PORT);
    srcAddr.sin_addr.s_addr = INADDR_ANY;
    int res = bind(s, (const struct sockaddr *)&srcAddr, sizeof(srcAddr));
    if (res == SOCKET_ERROR) {
        printf("Error: %d", WSAGetLastError());
        return false;
    }

    //Specify the destination address
    struct sockaddr_in dstAddr;
    int dstAddrSize = sizeof(dstAddr);
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port = htons(LOCAL_PORT);
    const char* remoteAddrStr = "127.0.0.1";
    inet_pton(AF_INET, remoteAddrStr, &dstAddr.sin_addr);

    //INFO: Send messages
    char sendBuf[BUF_LEN] = "pong\0";
    char recvBuf[BUF_LEN];

    for (int i = 0; i < 5; ++i) {
        memset(recvBuf, '/0', BUF_LEN);
        int recvLen = recvfrom(s, recvBuf, BUF_LEN, 0, (SOCKADDR*)&dstAddr, &dstAddrSize);
        if (recvLen == SOCKET_ERROR) {
            printf("Error: %d", WSAGetLastError());
            return false;
        }
        printf("Received packet from port %d\n", ntohs(dstAddr.sin_port));
        printf("Data: %s\n", recvBuf);
        sendto(s, sendBuf, BUF_LEN, 0, (SOCKADDR*)&dstAddr, sizeof(dstAddrSize));
        //if ( == SOCKET_ERROR) {
        //    printf("Error: %d", WSAGetLastError());
        //    return false;
        //}
    }

    //INFO: Clean up
    closesocket(s);
    WSACleanup();
    return 0;
}

//void printWSErrorAndExit(const char* msg)
//{
//    wchar_t* s = NULL;
//    FormatMessageW(
//        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
//        | FORMAT_MESSAGE_IGNORE_INSERTS,
//        NULL,
//        WSAGetLastError(),
//        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//        (LPWSTR)&s,
//        0, NULL);
//    fprintf(stderr, "%s: %S\n", msg, s);
//    LocalFree(s);
//    system("pause");
//    exit(-1);
//}