//Server
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
        printf("Error");
        // Log and handle error
        return false;
    }
    
    //INFO: Socket creation
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        printf("Error");
        //printWSErrorAndExit();
    }

    //Bind our adress (to listen to incoming connections)
    struct sockaddr_in srcAddr;
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_port = htons(LOCAL_PORT);
    srcAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    int res = bind(s, (const struct sockaddr *)&srcAddr, sizeof(srcAddr));

    //Specify the destination address
    struct sockaddr_in dstAddr;
    int dstAddrSize = sizeof(dstAddr);
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port = htons(LOCAL_PORT);
    const char* remoteAddrStr = "localhost";
    inet_pton(AF_INET, remoteAddrStr, &dstAddr.sin_addr);

    //INFO: Send messages
    const int bufLen = 1024;
    char sendBuf[bufLen] = "pong";
    char recvBuf[bufLen];

    for (int i = 0; i < 5; ++i) {
        recvfrom(s, recvBuf, bufLen, 0, (SOCKADDR*)&dstAddr, &dstAddrSize);
        printf(recvBuf);
        sendto(s, sendBuf, bufLen, 0, (SOCKADDR*)&dstAddr, sizeof(dstAddrSize));
    }
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