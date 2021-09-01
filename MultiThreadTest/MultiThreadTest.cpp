#include <windows.h>
#include <process.h>
#include <iostream>
#include <string>
#include <ppl.h>
#include <concurrent_queue.h>



// Cuncurrency safe queue object
concurrency::concurrent_queue<std::string>(gMessageQueue);

const std::string ExitCommand = "end";

unsigned int __stdcall thred2Function(void* p) {
	HANDLE* hEvent = (HANDLE*)p;

	while (1) {
		std::string message;

		WaitForSingleObject(hEvent, INFINITE);
		if(gMessageQueue.try_pop(message)) {
			std::cout << "received: " << message << std::endl;
			if (message == ExitCommand) {
				break;
			}
			Sleep(5000);
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

	DWORD threadId;
	HANDLE hHandle = (HANDLE)_beginthreadex(
		NULL,	// Security Attribute
		0,		// Stack size
		thred2Function, // Start address
		&hEvent,	// arg list
		0,		// initial state  0
		(unsigned int*)&threadId
	);

	if (!hHandle) {
		std::cout << "_beginthreadex fails !\n";
//		printf("_beginthreadex fails !\n");
		return 1;
	}

	while (1) {
		std::string input;
		std::getline(std::cin, input);

		gMessageQueue.push(input);
		std::cout << "SetEvent() returns " << SetEvent(hEvent) << std::endl;
		if (input == ExitCommand) {
			break;
		}
	}

	WaitForSingleObject(hHandle, INFINITE);
	CloseHandle(hHandle);
	CloseHandle(hEvent);
	return 0;
}