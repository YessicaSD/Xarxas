//Server
#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

int main(int argc, char** argv) {
	printf("Hello, this is the UDP server!\n");

	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	system("pause");
	return 0;
}