#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <functional>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Middleware.h"
#include "DetectionSystem.h"
#include <wiringPi.h>
//#include <cstddef.h>
#include "motor.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

std::vector<std::string> grayCircles;
std::vector<std::string> Radius20;
std::vector<std::string> allCircles;
char* nextImage;
int next = 0;

cv::Mat frame;
double timer;
Vector2 droneCartesianCoord;

unsigned int FPS; //frames per second
float FieldOfView; //Field of view

bool predictVelocity = false;
bool frameCompensation = true;
bool noiseDampening = false;

//[Range(0f, 180f)]
float velocityMaxDegreeChange = 1;
float cosOfMaxDegreeChange;


Vector2 velocity = Vector2(0, 0);
Vector2 prevTargetPos = Vector2(0, 0);

Vector2 SCREENSIZE;
Vector2 OFFSET_CAM;

bool isFirstRotation = true;
bool DroneWasDetectedOnThisFrame;
int cyclesSinceLastDetectionOfDrone = 0;

bool CalibrationMode = 0;
Vector2 CalibrationModeAngles;

int trackerType = 0;

const double DegToRad = M_PI / 180;
const int clockwise = -1, anticlockwise = 1;

StepperMotors *motors = NULL;

int main()
{
	Start();

    SetInput();
}

void Start() {
	//Read parameters from Middleware_Config.txt file
	std::string line;
	std::vector<std::string> lines;

	std::string fileName = "Middleware_Config.txt";
	std::ifstream myfile;
	myfile.open(fileName);

	if (!myfile.is_open()) {
		perror("Error open");
		exit(EXIT_FAILURE);
	}

	while (std::getline(myfile, line)) {
		if (line == "" || line.find("//") != std::string::npos) {
			continue;
		}
		//Grab everything after the '=' sign
		std::size_t found = line.find('=');
		if (found != std::string::npos) { //if line '=' found in current line then save line
			lines.push_back(line.substr(found + 1, line.length() - 1));
		}
	}

	//Make sure all variables in the config file are being set here.
	int n = 0;
	FPS = stoi(lines[n++]);
	FieldOfView = stoi(lines[n++]);
	predictVelocity = stoi(lines[n++]);
	frameCompensation = stoi(lines[n++]);
	noiseDampening = stoi(lines[n++]);
	velocityMaxDegreeChange = stoi(lines[n++]);
	SCREENSIZE.x = stoi(lines[n++]);
	SCREENSIZE.y = stoi(lines[n++]);
	OFFSET_CAM.x = stoi(lines[n++]);
	OFFSET_CAM.y = stoi(lines[n++]);
	CalibrationMode = stoi(lines[n++]);
	CalibrationModeAngles.x = atof(lines[n++].c_str());
	CalibrationModeAngles.y = atof(lines[n++].c_str());
	trackerType = stoi(lines[n]);

	if(n + 1 == lines.size()) {
		std::cout << "Succesfully read parameters from " << fileName << std::endl;
	}
	else if(n + 1 <= lines.size()) {
		std::cout << "Warning! It seems " << fileName << " has more parameters than assigned here." << std::endl;
	}
	else {
		std::cout << "Error " << fileName << " is missing parameters. Ending program." << std::endl;
		exit(EXIT_FAILURE);
	}

	wiringPiSetup();
	motors = new StepperMotors();
}

void SetInput()
{
    cv::namedWindow("Image", (640, 480));

    //Read parameters from Testing_Data.txt file
	std::string line;
	std::vector<std::string> lines;

	std::string fileName = "Testing_Data.txt";
	std::ifstream file;
	file.open(fileName);

	if (!file.is_open()) {
		perror("Error open");
		exit(EXIT_FAILURE);
	}

	while (std::getline(file, line))
	{
        if (line == "" || line.find("//") != std::string::npos)
        {
            continue;
        }

        lines.push_back(line.substr());
	}

	// Separates images by size and/or color for variable testing
    for (int i = 0; i < lines.size(); i++)
    {
        if (lines.at(i).at(12) == 'P' && lines.at(i).at(13) != 'n')
        {
            grayCircles.push_back(lines.at(i));
        }
        if (lines.at(i).at(13) == '2' || lines.at(i).at(14) == '2')
        {
            Radius20.push_back(lines.at(i));
        }

        allCircles.push_back(lines.at(i));
    }

    CallNextFrame(FixedUpdate, FPStoMilliseconds(FPS));

    while (std::cin.get() != '\n')
	{
		//Break loop if return key is pressed
	}

    // Run separate blocks for different testing variables (circle color, radius, etc)
    // Testing: gray circle color
    /*for (int i = 0; i < grayCircles.size(); i++)
    {
        nextImage = grayCircles.at(i);
        CallNextFrame(FixedUpdate, FPStoMilliseconds(FPS));
    }
    // Testing: 20 pixel radius
    for (int i = 0; i < Radius20.size(); i++)
    {
        nextImage = Radius20.at(i);
        CallNextFrame(FixedUpdate, FPStoMilliseconds(FPS));
    }

    // Testing: all images
    for (int i = 0; i < allCircles.size(); i++)
    {
        nextImage = allCircles.at(i);
        CallNextFrame(FixedUpdate, FPStoMilliseconds(FPS));
    }*/
}

void CallNextFrame(std::function<void(void)> func, unsigned int interval)
{
	std::thread([func, interval]()
		{
			while (true)
			{
                // Starts timer for FPS count
                timer = double(cv::getTickCount());

				auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
				func();
				std::this_thread::sleep_until(x);

                // Calculate frame rate
                float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);
                // Display fps in window
                cv::putText(frame, ("FPS: " + std::to_string(int(fps))), cv::Point(75,40), cv::FONT_HERSHEY_SIMPLEX, 0.7, (57, 255, 20), 2);
                // Display video on screen
				imshow("Image", frame);
                cv::waitKey(1);
			}
		}).detach();
}

void FixedUpdate()
{
	std::cout << "New Image Starts"<< std::endl;

    // Grabs next image
    if (next == allCircles.size())
        exit(0);
    nextImage = strdup(allCircles.at(next++).c_str());
    frame = cv::imread(nextImage);
    // Checks if frame was accessed
    if (frame.empty())
    {
        std::cout << "Error: Unable to grab image" << std::endl;
        exit(0);
    }

	//Vector2 droneCartesianCoord;
	bool onScreen;

	if (CalibrationMode)
	{
        RotateTowards(OFFSET_CAM, FieldOfView, SCREENSIZE);
        return;
	}
	else
	{
        onScreen = FindDrone(frame, trackerType);
	}

	// This calc works fine
	Vector2 center = Vector2(SCREENSIZE.x / 2, SCREENSIZE.y / 2); //Can be moved to Start() but screen size might change in the future
    std::cout << "Drone coord = (" << droneCartesianCoord.x << ", " << droneCartesianCoord.y << ")" << std::endl;
	Vector2 targetPosition = droneCartesianCoord - center; // If droneCartesiannCoord's center is at the bottom left corner, then shift to center
    std::cout << "1: tx = " << targetPosition.x << "and ty = " << targetPosition.y << std::endl;

	DroneWasDetectedOnThisFrame = true; //defualt flag to true
	if (!onScreen)
	{
		isFirstRotation = true; //If not on screen then set this flag to true
		DroneWasDetectedOnThisFrame = false;
		cyclesSinceLastDetectionOfDrone++;

		if (cyclesSinceLastDetectionOfDrone > FPS) { //lost visual for more than 1 second
			isFirstRotation = true;
			velocity = Vector2(0, 0); //reset velocity vector to (0,0)
		}
	}


	if (DroneWasDetectedOnThisFrame) {

		if (noiseDampening && prevTargetPos != Vector2(0, 0)) //If prevTargetPos = 0 (or minimal) then object is not currently moving so no restrictions on movement direction
		{
			targetPosition = ReduceNoice(targetPosition, prevTargetPos);
		}

		if (predictVelocity && !isFirstRotation)
		{
			if (cyclesSinceLastDetectionOfDrone > 1 && frameCompensation)
			{
                Vector2 lastPoint = velocity * -cyclesSinceLastDetectionOfDrone; // Travel back to position of last seen drone
                velocity = (targetPosition - lastPoint) / (cyclesSinceLastDetectionOfDrone + 1); // Assuming no acceleration
			}
			else
			{
                // Divide targetPosition by cyclesSinceLastDetectionOfDrone to get the new
                velocity = (targetPosition / cyclesSinceLastDetectionOfDrone) + velocity;
			}
			RotateTowards(targetPosition + velocity + OFFSET_CAM, FieldOfView, SCREENSIZE);
			prevTargetPos = targetPosition + velocity;
		}
		else {
			isFirstRotation = false;
			RotateTowards(targetPosition + OFFSET_CAM, FieldOfView, SCREENSIZE);
			prevTargetPos = targetPosition;
		}

		cyclesSinceLastDetectionOfDrone = 1; //Reset counter. This need to be reset AFTER the velocity is calculated for the current frame.
	}
    else // Drone was NOT detected on this frame
    {
        // Move to future position assuming constant velocity
        RotateTowards(velocity + OFFSET_CAM, FieldOfView, SCREENSIZE);
    }

	std::cout << "New Frame Ends"<< std::endl;
}

void RotateTowards(Vector2 targetPosition, float fieldOfView, Vector2 screenSize)
{
	float angleX = targetPosition.x * fieldOfView / screenSize.x;
	float angleY = targetPosition.y * fieldOfView / screenSize.y;

	if (CalibrationMode)
	{
        angleX = CalibrationModeAngles.x;
        angleY = CalibrationModeAngles.y;
	}

	std::cout << "X = " << angleX << " Y = " << angleY << std::endl;

    // Note: that angleX is the angle offset in the horizontal and angleY is vertical
    motors -> RotateMotors(Vector2(angleX, angleY));
}

Vector2 ReduceNoice(Vector2 targetPosition, Vector2 prev_targetPosition) {
	cosOfMaxDegreeChange = (float) cos(DegToRad * (velocityMaxDegreeChange));//field of vision to both sides of velocity vector.

	float cosOfAnglePhi = DotProduct2D(Normalized2D(prev_targetPosition), Normalized2D(targetPosition));

	//Debug.Log("Phi: " + Math.Acos(cosOfAnglePhi) / DegToRad + " < " + "MaxDegreeChange: " + Math.Acos(cosOfMaxDegreeChange) / DegToRad);
	if (cosOfAnglePhi > cosOfMaxDegreeChange) //Not inside view cone  CHECK THE SIGN
	{
		//Debug.Log("cosOfAnglePhi: " + cosOfAnglePhi + " > " + "cosOfMaxDegreeChange: " + cosOfMaxDegreeChange);
		Vector2 rotatedVectorCounterClockwise = Vector2();//vector in the edge of the field of vision (left of prev_targetPosition)
		Vector2 rotatedVectorClockwise = Vector2();//vector in the edge of the field of vision (right of prev_targetPosition)

		rotatedVectorCounterClockwise.x = (float)(prev_targetPosition.x * cos(DegToRad * velocityMaxDegreeChange) - prev_targetPosition.y * sin(DegToRad * velocityMaxDegreeChange));
		rotatedVectorCounterClockwise.y = (float)(prev_targetPosition.x * sin(DegToRad * velocityMaxDegreeChange) + prev_targetPosition.y * cos(DegToRad * velocityMaxDegreeChange));
		rotatedVectorClockwise.x = (float)(prev_targetPosition.x * cos(DegToRad * velocityMaxDegreeChange) + prev_targetPosition.y * sin(DegToRad * velocityMaxDegreeChange));
		rotatedVectorClockwise.y = (float)(prev_targetPosition.x * -sin(DegToRad * velocityMaxDegreeChange) + prev_targetPosition.y * cos(DegToRad * velocityMaxDegreeChange));

		if (cosOfAnglePhi > 0)
		{
			//Then angle is less than 90 degrees
		}
		else if (cosOfAnglePhi < 0) //Then angle is more than 90 degrees
		{
			//Flip vectors since targetPosition is on the opposite side
			rotatedVectorCounterClockwise = rotatedVectorCounterClockwise * -1;
			rotatedVectorClockwise = rotatedVectorClockwise * -1;
			//prev_targetPosition *= -1;
		}
		else//(cosOfAnglePhi == 0) Then angle is 90 degrees
		{
			isFirstRotation = true;
			return Vector2(0, 0); //Unlikely the the drone moved in this direction. Ignore this frame
		}

		//Get projection on the closest rotated vector
		float cosOfCounterClockwiseVector = DotProduct2D(Normalized2D(rotatedVectorCounterClockwise), Normalized2D(targetPosition));
		float cosOfClockwiseVector = DotProduct2D(Normalized2D(rotatedVectorClockwise), Normalized2D(targetPosition));

		if (cosOfCounterClockwiseVector > cosOfClockwiseVector)
		{
			//Then counter clockwise vector is closer
			return ProjectionOf_U_Onto_V(targetPosition, rotatedVectorCounterClockwise);
		}
		else
		{
			//Then clockwise vector is closer
			return ProjectionOf_U_Onto_V(targetPosition, rotatedVectorClockwise);
		}
	}
	else
	{
		//targetPosition is already in the direction of posible directions
		return targetPosition;
	}
}

float FPStoMilliseconds(unsigned int fps)
{
	return (float)(1000 / fps);
}

