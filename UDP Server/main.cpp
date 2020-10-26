//#pragma comment(lib, "ws2_32.lib")
#include "../Sockets/Helpers.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	WSADATA wsaData;
	Init(wsaData);

	printf("Hello this is the UPD Server /n");
	system("pause");

	CleanUp();
	return 0;
}