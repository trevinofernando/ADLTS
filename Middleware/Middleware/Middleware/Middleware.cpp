#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <Windows.h>

const unsigned int FPS = 1;

float FPStoMilliseconds(unsigned int fps);
void timer_start(std::function<void(void)> func, unsigned int interval);
void FixedUpdate();


int main()
{
	timer_start(FixedUpdate, FPStoMilliseconds(FPS));
	
	//Prevent the program from ending
	while (!GetAsyncKeyState(VK_RETURN))
	{	
		//Break loop if return key is pressed
	}
}

void timer_start(std::function<void(void)> func, unsigned int interval)
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
