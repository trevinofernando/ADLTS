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
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking/tracker.hpp>
#include "motor.h"
#include <wiringPi.h>

using namespace std;
using namespace cv;

VideoCapture cap;
Mat frame, bgFrame;
double timer;
Vector2 droneCartesianCoord;
float FieldOfView;
Ptr<Tracker> tracker;
int trackerNum = 4;
Rect2d bbox;
bool isCircle = false;
int bboxDim;
Vector2 center;
Vector2 targetPosition;
Vector2 prevPosition;
float degreesPerPixel;
float angleX;
float angleY;
Vector2 currMotorAngles;
Rect2d centerFrame;
bool nearCenter = false;
Scalar centerBoxColor = Scalar(0, 0, 255);

// Only values you would need to change
Vector2 Screensize = Vector2(640, 480);
float maxAngle = 1;
bool limitInitDistance = false;

StepperMotors *motor = NULL;

void Init();
void Detect();
void Track();
void RotateTowards();
void ResetMotors();

int main()
{
    int frameCount = 0;
    Init();

    cout << "Initializing detection..." << endl;

    while (true)
    {
        timer = double(getTickCount());

        cap >> frame;
        frameCount++;
        if (frame.empty())
        {
            exit(0);
        }


        if (!isCircle)
        {
            Detect();
        }
        else
        {

            if (limitInitDistance)
            {
                if (nearCenter)
                {
                    centerBoxColor = Scalar(0, 255, 0);

                    if ((abs(prevPosition.x - targetPosition.x) > 15) || (abs(prevPosition.y - targetPosition.y) > 15))
                    {
                        cout << "Rotating" << endl;
                        RotateTowards();
                    }
                }
                else
                {
                    centerBoxColor = Scalar(0, 0, 255);
                }
            }
            else
            {
                if ((abs(prevPosition.x - targetPosition.x) > 15) || (abs(prevPosition.y - targetPosition.y) > 15))
                {
                    cout << "Rotating" << endl;
                    RotateTowards();
                }
            }

            Track();
        }

        if (limitInitDistance)
            rectangle(frame, centerFrame, centerBoxColor, 2, 1);

        circle(frame, Point(320, 240), 1, Scalar(0, 255, 0), 2);

        float fps = getTickFrequency() / (double(getTickCount()-timer));
        putText(frame, ("FPS: " + std::to_string(int(fps))), Point(75,40), FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 255), 2);

        imshow("Frame", frame);
        if (waitKey(1) == 27)
        {
            ResetMotors();
            cout << frameCount << endl;
            exit(0);
        }
    }

}

void Init()
{
    string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW",
    "MOSSE", "CSRT"};

    string trackerType = trackerTypes[trackerNum];

    if (trackerType == "BOOSTING")
        tracker = TrackerBoosting::create();
    else if (trackerType == "MIL")
        tracker = TrackerMIL::create();
    else if (trackerType == "KCF")
        tracker = TrackerKCF::create();
    else if (trackerType == "TLD")
        tracker = TrackerTLD::create();
    else if (trackerType == "MEDIANFLOW")
        tracker = TrackerMedianFlow::create();
    else if (trackerType == "MOSSE")
        tracker = TrackerMOSSE::create();
    else if (trackerType == "CSRT")
        tracker = TrackerCSRT::create();

    cout << "Tracker: " << trackerType << endl;

    cap.open(0);
    if (!cap.isOpened())
    {
        cerr << "ERROR: Unable to open camera, exiting..." << endl;
        exit(0);
    }

    //cap.set(cv::CAP_PROP_FPS, 30);
    cap.set(3, Screensize.x);
    cap.set(4, Screensize.y);

    // Middleware initialize
    FieldOfView = 62;

    center = Vector2(Screensize.x / 2, Screensize.y / 2);
    degreesPerPixel = FieldOfView / Screensize.x;

    wiringPiSetup();
	motor = new StepperMotors();

	currMotorAngles.x = 0;
	currMotorAngles.y = 0;

	// As of 11/14, if object is at outer edge of the frame, the motors attempt to center too quickly throwing off the tracker
	// Limiting motor movement per cycle didnt help much.
	// Create box for allowed motor movement
	centerFrame.x = Screensize.x / 4;
	centerFrame.y = Screensize.y / 4;
	centerFrame.width = Screensize.x / 2;
	centerFrame.height = Screensize.y / 2;
}

void Detect()
{
    Mat gray, blur;
    vector<Vec3f> circles;

    auto start = chrono::high_resolution_clock::now();

    cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    GaussianBlur(gray, blur, Size(9,9), 2, 2);

    HoughCircles(blur, circles, HOUGH_GRADIENT, 1, frame.rows/8, 25, 50, 5, 75);

    if (circles.size() > 0)
    {
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> detectTime = (end - start)/1000;
        cout << "Time to detect: " << detectTime.count() << "seconds" << endl;

        isCircle = true;
        droneCartesianCoord.x = float(cvRound(circles[0][0]));
        droneCartesianCoord.y = float(cvRound(circles[0][1]));

        if (droneCartesianCoord.x >= centerFrame.x && droneCartesianCoord.x <= (centerFrame.x + centerFrame.width))
        {
            if (droneCartesianCoord.y >= centerFrame.y && droneCartesianCoord.y <= (centerFrame.y + centerFrame.height))
                nearCenter = true;
            else
                nearCenter = false;
        }
        else
        {
            nearCenter = false;
        }

        // Inits previous position
        targetPosition.x = droneCartesianCoord.x - center.x;
        targetPosition.y = -droneCartesianCoord.y + center.y;

        int radius = cvRound(circles[0][2]);
        cout << "Radius = " << radius << endl;

        bboxDim = 2 * (cvRound(circles[0][2]));
        Rect2d box(cvRound(circles[0][0]) - radius, cvRound(circles[0][1]) - radius, bboxDim, bboxDim);
        bbox = box;

        // Draw circle on image
        Point centroid(droneCartesianCoord.x, droneCartesianCoord.y);
        circle(frame, centroid, radius, Scalar(255, 0, 0), 2, 1);

        // Initialize tracker
        tracker->init(frame, bbox);
        circles.clear();
    }
}

void Track()
{

    Rect2d prevbbox = bbox;

    if (tracker->update(frame,bbox))
    {
        cout << "Tracking" << endl;
    }
    else
    {
        cout << "Not tracking" << endl;
    }

    /*if (bbox.width > 1.5*prevbbox.width || bbox.height > 1.5*prevbbox.height)
    {
        cout << "Lost object" << endl;
        isCircle = false;
    }*/

    rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

    droneCartesianCoord.x = bbox.x + bbox.width/2;
    droneCartesianCoord.y = bbox.y + bbox.height/2;

    if (droneCartesianCoord.x >= centerFrame.x && droneCartesianCoord.x <= (centerFrame.x + centerFrame.width))
    {
        if (droneCartesianCoord.y >= centerFrame.y && droneCartesianCoord.y <= (centerFrame.y + centerFrame.height))
            nearCenter = true;
        else
            nearCenter = false;
    }
    else
    {
        nearCenter = false;
    }

    Vector2 offset;
    offset.x = 0;
    offset.y = bbox.height/2;

    targetPosition.x = droneCartesianCoord.x - center.x + offset.x;
    targetPosition.y = -droneCartesianCoord.y + center.y - offset.y;

    cout << "Drone Coord: (" << droneCartesianCoord.x << ", " << droneCartesianCoord.y << ")" << endl;
    cout << "Target Pos = " << targetPosition.x << ", " << targetPosition.y << endl;
}

void RotateTowards()
{
    angleX = -1 * targetPosition.x * degreesPerPixel;
    angleY = -1 * targetPosition.y * degreesPerPixel;

    int xDir, yDir;

    if (angleX < 0)
        xDir = -1;
    else
        xDir = 1;

    if (angleY < 0)
        yDir = -1;
    else
        yDir = 1;

    if (abs(angleX) > maxAngle)
        angleX = xDir * maxAngle;
    if (abs(angleY) > maxAngle)
        angleY = yDir * maxAngle;

    currMotorAngles.x += angleX;
    currMotorAngles.y += angleY;

    cout << "Angles: (" << angleX << ", " << angleY << ")" << endl;


    motor->RotateMotors(Vector2(angleX, angleY));
}

void ResetMotors()
{
    // Reverse the current position to reset motor position
    currMotorAngles.x = -1 * currMotorAngles.x;
    currMotorAngles.y = -1 * currMotorAngles.y;

    motor->RotateMotors(currMotorAngles);
}
