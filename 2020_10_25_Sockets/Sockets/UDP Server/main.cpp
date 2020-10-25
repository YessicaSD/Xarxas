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

    //TODO: Open the socket to start listening for connections and packages?
    struct sockaddr_in bindAddr;
    bindAddr.sin_family = AF_INET; 

    bindAddr.sin_port = htons(LOCAL_PORT); // Port
    bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

    system("pause");

    //Receive a message
    const int bufLen = 1024;
    char buf[bufLen];
    
    struct sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);

    recvfrom(s, buf, bufLen, 0, (SOCKADDR*)&senderAddr, &senderAddrSize);
}

void printWSErrorAndExit(const char* msg)
{
    wchar_t* s = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&s,
        0, NULL);
    fprintf(stderr, "%s: %S\n", msg, s);
    LocalFree(s);
    system("pause");
    exit(-1);
}