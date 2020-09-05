#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/tracking/tracker.hpp>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
	Rect2d roi;
	Mat frame;
	double timer;

	// Sets first connected camera as source camera for OpenCV
	VideoCapture src = cv::VideoCapture(0);

	// Defines tracker using MOSSE tracking algorithm
	Ptr<Tracker> tracker = cv::TrackerMOSSE::create();

	// Closes app if camera doesn't open
	if (!src.open(0))
	{
		cout << "Error: Webcam didn't open.";
		return 0;
	}

	// Grab initial frame for bounding box selection
	src >> frame;
	// Allows user to click and drag bounding box
	roi = selectROI("tracker", frame);
	// Initializes the tracker using the given bounding box
	tracker->init(frame, roi);


	while (true)
	{
		// Starts time for FPS counter
		timer = double(cv::getTickCount());

		// Grabs next frame
		src >> frame;

		//Updates tracker using last bounding box coordinates
		tracker->update(frame, roi);
		// Draws rectangle in frame around the box dimensions
		rectangle(frame, roi, Scalar(255, 0, 0), 2, 1);

		// Calculates current FPS 
		float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);
		// Displays FPS in upper left corner of frame
		cv::putText(frame, ("FPS: " + std::to_string(int(fps))), Point(75, 40), cv::FONT_HERSHEY_COMPLEX, 0.7, (20, 230, 20), 2);

		// Displays camera frame
		imshow("tracker", frame);

		// Exits application if ESC is pressed
		if (waitKey(1) == 27)
			break;
	}

	return 0;
}
