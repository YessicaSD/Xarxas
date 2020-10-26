
//#pragma comment(lib, "ws2_32.lib")
#include "Helpers.h"

#include <stdlib.h>





int main(int argc, char** argv)
{
	WSADATA wsaData;

	Init(wsaData);

	printf("Hello this is the UPD Client");
	system("pause");

	int iResult = WSACleanup();
	if (iResult != NO_ERROR)
	{
		
	}
	return 0;
}

