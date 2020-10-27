//#pragma comment(lib, "ws2_32.lib")
#include "../Sockets/Helpers.h"

#include <stdlib.h>
#include <stdio.h>



int main(int argc, char** argv)
{
	WSADATA wsaData;
	Init(wsaData);
	SOCKET s;
	CreateUDPSocket(s);
	
	int enable = 1;
	int res = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));

	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("Error configured socket");
	}

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(port); // Port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address
	const char* remoteAddrStr = "127.0.0.1 ";
	inet_pton(AF_INET, remoteAddrStr, &bindAddr.sin_addr);

	res = bind(s, (const struct sockaddr*) & bindAddr, sizeof(bindAddr));

	const char* buf = "pong";
	char recieved_msg[MAX_MESSAGE_SIZE];
	int sizeAddr = sizeof(bindAddr);

	printf("Hello this is the UPD Server /n");
	while (true)
	{
		int res = recvfrom(s, recieved_msg, MAX_MESSAGE_SIZE, 0, (sockaddr*)&bindAddr, &sizeAddr);

		if (res == SOCKET_ERROR)
			printWSErrorAndExit("Message not recieved!");
		else
		{
			recieved_msg[res] = '\0';
			printf_s("%s\n", recieved_msg);
			Sleep(1000);
		}

		// Send message to client
		res = sendto(s, buf, sizeof(buf), 0, (sockaddr*)(&bindAddr), sizeAddr);
		if (res == SOCKET_ERROR)
			printWSErrorAndExit("Message don't send!");
	}

	



	
	system("pause");

	CleanUp(s);
	return 0;
}