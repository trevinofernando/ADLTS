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

cv::VideoCapture cap;
cv::Mat frame;
double timer;

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

	CallNextFrame(FixedUpdate, FPStoMilliseconds(FPS));

	//Prevent the program from ending
	while (std::cin.get() != '\n')
	{
		//Break loop if return key is pressed
	}
}

void Start() {
    // Opens camera module
    cap.open(0);
    // Checks if camera opened successfully
    if (!cap.isOpened())
    {
        std::cerr << "ERROR: Unable to open the camera" << std::endl;
        exit(0);
    }

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

	//Make sure all variables in the config file are beeing set here.
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
	trackerType = stoi(lines[n++]);

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
				imshow("Live Feed", frame);
                cv::waitKey(0);
			}
		}).detach();
}

void FixedUpdate()
{
	std::cout << "New Frame Starts"<< std::endl;

    // Grabs frame from camera
    cap >> frame;
    // Checks if frame was accessed
    if (frame.empty())
    {
        std::cerr << "Error: Unable to grab from the camera" << std::endl;
        exit(0);
    }
    flip(frame, frame, 0);

	Vector2 droneCartesianCoord;
	bool onScreen;

	if (CalibrationMode)
	{
        RotateTowards(OFFSET_CAM, FieldOfView, SCREENSIZE);
        return;
	}
	else
	{
        onScreen = FindDrone(droneCartesianCoord, frame, trackerType);
	}

	Vector2 center = Vector2(SCREENSIZE.x / 2, SCREENSIZE.y / 2); //Can be moved to Start() but screen size might change in the future
	Vector2 targetPosition = droneCartesianCoord - center; // If droneCartesiannCoord's center is at the bottom left corner, then shift to center


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
                Vector2 lastPoint;
                lastPoint.x = -velocity.x * cyclesSinceLastDetectionOfDrone; // Travel back to position of last seen drone
                lastPoint.y = -velocity.y * cyclesSinceLastDetectionOfDrone; // Travel back to position of last seen drone
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

	std::cout << "tx = " << targetPosition.x << "and ty = " << targetPosition.y << std::endl;
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

