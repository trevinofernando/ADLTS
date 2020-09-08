#include <iostream>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking/tracker.hpp>
#include "DetectionSystem.h"

using namespace std;
using namespace cv;

VideoCapture cap;
Mat frame;
Ptr<Tracker> tracker;
Rect2d bbox;

bool FindDrone(Vector2& droneCartesiannCoord)
{
	droneCartesiannCoord = Vector2(0, 0);
	return true; //If drone was detected
}
