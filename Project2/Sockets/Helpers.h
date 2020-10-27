#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 
#include "WinSock2.h"
#include "Ws2tcpip.h"
#include <stdio.h>
#include <stdlib.h>
u_short port = 8000;

#define MAX_MESSAGE_SIZE 1024


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

void SendTo(SOCKET& s, const char* messatge, sockaddr_in& address)
{
	sendto(s, messatge, sizeof(messatge), 0, (const sockaddr*)&address, sizeof(address));
}

bool Init(WSADATA& wsaData)
{
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		// Log and handle error
		printWSErrorAndExit("Error Init;");
		return false;
	}
	return true;
}

/*
	directions ( SD_RECEIVE, SD_SEND, SD_BOTH)
	int shutdown ( SOCKET s, int direction );
*/
bool CleanUp(SOCKET& s)
{
	closesocket(s);

	int iResult = WSACleanup();
	if (iResult != NO_ERROR)
	{
		// Log and handle error
		printWSErrorAndExit("Error CleanUp;");
		return false;
	}
	return true;
}


//AF_INET refers to the family of addresses IPv4
// IPv6 address family were needed, we should pass, the value AF_INET6 .

/*
	With SOCK_DGRAM (for UDP sockets) discrete datagrams (that is, independent
	packets of data) will be sent through the network. With SOCK_STREAM (for TCP sockets)
	the data sent through the network will be packed into a contiguous stream of data that
	needs to be separated by the application code.
*/
bool CreateUDPSocket(SOCKET& s)
{
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		printWSErrorAndExit("Failed to create UDP");
		return false;
	}
	return true;
}

bool CreateTCPSocket(SOCKET& s)
{
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		printWSErrorAndExit("Failed to create TCP");
		return false;
	}
	return true;
}