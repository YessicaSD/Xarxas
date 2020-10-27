
//#pragma comment(lib, "ws2_32.lib")
#include "Helpers.h"

#include <stdlib.h>





int main(int argc, char** argv)
{
	WSADATA wsaData;
	Init(wsaData);

	SOCKET s;
	CreateUDPSocket(s);
	printf("Hello this is the UPD Client");
	
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(port); // Port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address
	const char* remoteAddrStr = "127.0.0.1 ";
	inet_pton(AF_INET, remoteAddrStr, &bindAddr.sin_addr);

	const char* buf = "ping";
	char recieved_msg[MAX_MESSAGE_SIZE];
	int sizeAddr = sizeof(bindAddr);
	while (true)
	{

		sendto(s, buf, sizeof(buf), 0, (const sockaddr*)&bindAddr, sizeAddr);

		int res = recvfrom(s, recieved_msg, MAX_MESSAGE_SIZE, 0, (sockaddr*)&bindAddr, &sizeAddr);

		if (res == SOCKET_ERROR)
		{
			printWSErrorAndExit("Error reciving messatge");
		}
		else
		{
			recieved_msg[res] = '\0';
			printf_s("%s\n", recieved_msg);
			Sleep(1000);
		}
	}
	
	system("pause");



	int iResult = WSACleanup();
	if (iResult != NO_ERROR)
	{
		
	}
	return 0;
}

