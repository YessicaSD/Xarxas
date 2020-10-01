#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <mutex>


long long int myGlobal = 0;
std::mutex mtx;
void  function()
{
	for (long long int i = 0; i < 10000; ++i)
	{
		std::unique_lock < std::mutex> lock(mtx);
		++myGlobal;
	}

}
int main()
{
	std::thread t[2] = {
		std::thread(function),
		std::thread(function) };


	t[0].join();
	t[1].join();
	printf("Global: %lld \n", myGlobal);
	
	system("pause");

	return 0;
}