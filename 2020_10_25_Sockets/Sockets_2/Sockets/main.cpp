//Client
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"
#include <stdio.h>
#include <stdlib.h>

#define LOCAL_PORT 8000

int main(int argc, char** argv)
{
    //INFO: Initialization
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        // Log and handle error
        return false;
    }

    //INFO: Socket creation
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        printf("Error");
        //printWSErrorAndExit();
    }
        
    //INFO: Connect the socket
    struct sockaddr_in dstAddr;
    int dstAddrSize = sizeof(dstAddr);
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port = htons(LOCAL_PORT);
    const char* remoteAddrStr = "localhost";
    inet_pton(AF_INET, remoteAddrStr, &dstAddr.sin_addr);

    //INFO: Send information
    const int bufLen = 1024;
    char sendBuf[bufLen] = "ping";
    char recvBuf[bufLen];

    for (int i = 0; i < 5; ++i) {
        sendto(s, sendBuf, bufLen, 0, (SOCKADDR *) &dstAddr, sizeof(dstAddr));
        recvfrom(s, recvBuf, bufLen, 0, (SOCKADDR *) &dstAddr, &dstAddrSize);
        printf(recvBuf);
    }

    //INFO: Clean up
    //WSACleanup();

    system("pause");
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