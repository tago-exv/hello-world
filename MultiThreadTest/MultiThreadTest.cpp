#include <windows.h>
#include <process.h>
#include <iostream>
#include <string>
#include <thread>
#include <ppl.h>
#include <concurrent_queue.h>



// Cuncurrency safe queue object
concurrency::concurrent_queue<std::string>(gMessageQueue);

const std::string ExitCommand = "end";


unsigned int thred2Function(void* p) {
	HANDLE hEvent = *(HANDLE*)p;

	while (1) {
		std::string message;

		WaitForSingleObject(hEvent, INFINITE);
		std::cout << "Catch the event\n";
		if(gMessageQueue.try_pop(message)) {
			std::cout << "received: " << message << std::endl;
			if (message == ExitCommand) {
				break;
			}
			Sleep(5000);
		}
		else {
			ResetEvent(hEvent);
		}
	}
	return 0;
}

int main()
{
	printf("Multi Thread Test 1\n");

	// Event object
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("EVENT"));
	if (!hEvent) {
		std::cout << "CreateEvent fails !\n";
		return 1;
	}

	std::thread thr1(thred2Function, &hEvent);

	while (1) {
		std::string input;
		std::getline(std::cin, input);

		gMessageQueue.push(input);
	
	std::cout << "SetEvent() returns " << SetEvent(hEvent) << std::endl;
		if (input == ExitCommand) {
			break;
		}
	}

	thr1.join();
	CloseHandle(hEvent);
	return 0;
}