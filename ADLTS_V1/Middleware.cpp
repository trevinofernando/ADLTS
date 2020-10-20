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
#include <opencv2/opencv.hpp>
#include "DetectionSystem.h"
#include <wiringPi.h>
//#include <cstddef.h>
#include "motor.h"

#define DEADZONE 75

using namespace std;

bool completeRotation = true;

cv::VideoCapture cap;
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
Vector2 prevVelocity = Vector2(0, 0);
Vector2 prevTargetPos = Vector2(0, 0);

Vector2 SCREENSIZE;
Vector2 OFFSET_CAM;
Vector2 MotorsDir;

bool isFirstRotation = true;
bool DroneWasDetectedOnThisFrame;
int cyclesSinceLastDetectionOfDrone = 0;

bool CalibrationMode = 0;
Vector2 CalibrationModeAngles;

int trackerType = 0;
bool canTrack = false; //If the drone is expected to be far away from the previous position then we need detect the drone. can't track
float maxTrackingDistance;

const double DegToRad = M_PI / 180;
const int clockwise = -1, anticlockwise = 1;

StepperMotors *motor = NULL;


int main()
{
	Start();

	CallNextFrame(FixedUpdate, FPStoMilliseconds(FPS));

	//Prevent the program from ending
	while (std::cin.get() != '\n')
	{
		//Break loop if return key is pressed
		//FixedUpdate();
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
	std::ifstream myFile;
	myFile.open(fileName);

	if (!myFile.is_open()) {
		perror("Error open");
		exit(EXIT_FAILURE);
	}

	while (std::getline(myFile, line)) {
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
	FPS = stoi(lines[n++]); //int
	FieldOfView = atof(lines[n++].c_str()); //float
	predictVelocity = stoi(lines[n++]); //bool
	frameCompensation = stoi(lines[n++]); //bool
	noiseDampening = stoi(lines[n++]); //bool
	velocityMaxDegreeChange = atof(lines[n++].c_str()); //float
	SCREENSIZE.x = stoi(lines[n++]); //int
	SCREENSIZE.y = stoi(lines[n++]); //int
	OFFSET_CAM.x = stoi(lines[n++]); //int
	OFFSET_CAM.y = stoi(lines[n++]); //int
	MotorsDir.x = atof(lines[n++].c_str()); //float
	MotorsDir.y = atof(lines[n++].c_str()); //float
	CalibrationMode = stoi(lines[n++]); //bool
	CalibrationModeAngles.x = atof(lines[n++].c_str()); //float
	CalibrationModeAngles.y = atof(lines[n++].c_str()); //float
	trackerType = stoi(lines[n++]); //int
	maxTrackingDistance = atof(lines[n].c_str()); //float

	if(n == lines.size()) {
		std::cout << "Successfully read parameters from " << fileName << std::endl;
	}
	else if(n <= lines.size()) {
		std::cout << "Warning! It seems " << fileName << " has more parameters than assigned here." << std::endl;
	}
	else {
		std::cout << "Error " << fileName << " is missing parameters. Ending program." << std::endl;
		exit(EXIT_FAILURE);
	}

	wiringPiSetup();
	motor = new StepperMotors();
}

void CallNextFrame(std::function<void(void)> func, unsigned int interval)
{
	std::thread([func, interval]()
		{
			while (true)
			{
				auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
                timer = double(cv::getTickCount());

                cap >> frame;
                if (frame.empty())
                    exit(0);

				func();

				std::this_thread::sleep_until(x);

				// Calculate frame rate
                float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);
                // Display fps in window
                cv::putText(frame, ("FPS: " + std::to_string(int(fps))), cv::Point(75,40), cv::FONT_HERSHEY_SIMPLEX, 0.7, (57, 255, 20), 2);

                // Display video on screen
				imshow("Live Feed", frame);
				if (cv::waitKey(1) == 27)
                    exit(0);
			}
		}).detach();
}

void FixedUpdate()
{
	std::cout << "New Frame Starts"<< std::endl;

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

	Vector2 center = Vector2(SCREENSIZE.x / 2, SCREENSIZE.y / 2); //Can be moved to Start() but screen size might change in the future

    Vector2 targetPosition;
	targetPosition.x = droneCartesianCoord.x - center.x; //subtract x for shifting
	targetPosition.y = -droneCartesianCoord.y + center.y; //subtract y for shifting then flip result for axis inversion
    if (onScreen)
    {
        cout << "Drone Coord: (" << droneCartesianCoord.x << ", " << droneCartesianCoord.y << ")" << endl;
        cout << "Target Pos = " << targetPosition.x << ", " << targetPosition.y << endl;
    }

	if(Distance2D(prevTargetPos, targetPosition) < DEADZONE){
		prevTargetPos = Vector2(0, 0); //No action
		return; //Kill Queue if prev position is withing a radius of [DEADZONE] pixels
	}

	DroneWasDetectedOnThisFrame = true; //default flag to true
	if (!onScreen)
	{
		isFirstRotation = true; //If not on screen then set this flag to true
		DroneWasDetectedOnThisFrame = false;
		cyclesSinceLastDetectionOfDrone++;

		if (cyclesSinceLastDetectionOfDrone > FPS)
		{ //lost visual for more than 1 second
			isFirstRotation = true;
			velocity = Vector2(0, 0); //reset velocity vector to (0,0)
			return; //skip this frame to stop turret from turning indefinitely until drone is detected again.
		}
		//Delete this return for final build:
		return;
	}


	if (DroneWasDetectedOnThisFrame)
	{

		if (noiseDampening && prevTargetPos != Vector2(0, 0)) //If prevTargetPos = 0 (or minimal) then object is not currently moving so no restrictions on movement direction
		{
			targetPosition = ReduceNoise(targetPosition, prevTargetPos);
		}

		if (predictVelocity && !isFirstRotation)
		{
			if(cyclesSinceLastDetectionOfDrone > 1 && frameCompensation)
			{
				Vector2 lastPoint = velocity * -cyclesSinceLastDetectionOfDrone; //Travel back to position of last seen drone
				velocity = (targetPosition - lastPoint) / (cyclesSinceLastDetectionOfDrone + 1); //Assuming no acceleration
			}
			else
			{
				//Divide targetPosition by cyclesSinceLastDetectionOfDrone to get the new
				velocity = (targetPosition / cyclesSinceLastDetectionOfDrone) + velocity;
			}
			RotateTowards(targetPosition + velocity + OFFSET_CAM, FieldOfView, SCREENSIZE);
			prevTargetPos = targetPosition + velocity;
		}
		else
		{
			isFirstRotation = false;
			canTrack = Distance2D(targetPosition, prevTargetPos) < maxTrackingDistance;
			RotateTowards(targetPosition + OFFSET_CAM, FieldOfView, SCREENSIZE);
			prevTargetPos = targetPosition;
		}

		cyclesSinceLastDetectionOfDrone = 1; //Reset counter. This need to be reset AFTER the velocity is calculated for the current frame.
	}
	else if(predictVelocity)//Drone Was NOT Detected On This Frame
	{
		//Move to future position assuming constant velocity
		RotateTowards(velocity + OFFSET_CAM, FieldOfView, SCREENSIZE);
	}

	std::cout << "New Frame Ends"<< std::endl;
}

void RotateTowards(Vector2 targetPosition, float fieldOfView, Vector2 screenSize)
{
	float degreesPerPixel = fieldOfView / screenSize.x;
	float angleX = MotorsDir.x * targetPosition.x * degreesPerPixel;
	float angleY = MotorsDir.y * targetPosition.y * degreesPerPixel;
	if (CalibrationMode)
	{
		angleX = CalibrationModeAngles.x;
		angleY = CalibrationModeAngles.y;
	}
	std::cout << "Angle X: " << angleX << std::endl << "Angle Y: " << angleY << std::endl;

	//while (!completeRotation)
	//{}
	//Note: that angleX is the angle offset in the horizontal and angleY is vertical
	//completeRotation = false;
	motor -> RotateMotors(Vector2(angleX, angleY));
}

Vector2 ReduceNoise(Vector2 targetPosition, Vector2 prev_targetPosition) {
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
		//targetPosition is already in the direction of possible directions
		return targetPosition;
	}
}

float FPStoMilliseconds(unsigned int fps)
{
	return (float)(1000 / fps);
}

