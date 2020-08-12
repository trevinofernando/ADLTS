#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <Windows.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Middleware.h"

unsigned int FPS = 1; //Choose the desired frames per second

bool predictVelocity = false;
bool noiseDampening = false;

//[Range(0f, 180f)]
float velocityMaxDegreeChange = 1;
float cosOfMaxDegreeChange;


Vector2 velocity;
Vector2 prevTargetPos = Vector2(0, 0);

Vector2 SCREENSIZE;

bool isFirstRotation = true;
bool DroneWasDetectedOnThisFrame;
int cyclesSinceLastDetectionOfDrone = 0;


const double DegToRad = M_PI / 180;
const int clockwise = -1, anticlockwise = 1;


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
	//Read parameters from Middleware_Config.txt file
	
	std::string line;
	std::vector<std::string> lines;

	std::ifstream myfile;
	myfile.open("Middlware_Config.txt");

	if (!myfile.is_open()) {
		perror("Error open");
		exit(EXIT_FAILURE);
	}

	while (std::getline(myfile, line)) {
		if (line._Equal("") || line.find("//") != std::string::npos) {
			continue;
		}
		//Grab everything after the '=' sign
		std::size_t found = line.find('=');
		if (found != std::string::npos) { //if line '=' found in current line then save line
			lines.push_back(line.substr(found + 1, line.length() - 1));
		}
	}

	//Make sure all variables in the config file are beeing set here.
	int n = 0;
	FPS = stoi(lines[n++]);
	predictVelocity = stoi(lines[n++]);
	noiseDampening = stoi(lines[n++]);
	velocityMaxDegreeChange = stoi(lines[n++]);
	SCREENSIZE.x = stoi(lines[n++]);
	SCREENSIZE.y = stoi(lines[n++]);
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
	std::cout << "I am doing something"<< std::endl;
}

float FPStoMilliseconds(unsigned int fps) 
{
	return (float)(1000 / fps);
}
