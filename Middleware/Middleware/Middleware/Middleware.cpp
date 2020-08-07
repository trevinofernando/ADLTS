#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <Windows.h>
#include "Middleware.h"

//using namespace std;
const unsigned int FPS = 1; //Choose the desired frames per second

int main()
{
	Start();

	CallNextFrame(FixedUpdate, FPStoMilliseconds(FPS));
	
	//Prevent the program from ending
	while (!GetAsyncKeyState(VK_RETURN))
	{	
		//Break loop if return key is pressed
	}
}

void Start() {
	//TODO: Read parameters from config file
}

void CallNextFrame(std::function<void(void)> func, unsigned int interval)
{
	std::thread([func, interval]()
		{
			while (true)
			{
				auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
				func();
				std::this_thread::sleep_until(x);
			}
		}).detach();
}

void FixedUpdate()
{
	std::cout << "I am doing something" << std::endl;
}

float FPStoMilliseconds(unsigned int fps) 
{
	return (float)(1000 / fps);
}
