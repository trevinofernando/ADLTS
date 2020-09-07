#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/tracking/tracker.hpp>
#include <iostream>
#include <stdio.h>


using namespace std;
using namespace cv;

#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

int main(int argc, char** argv)
{
	Mat frame, gray;
	int frameNum = 1;
	double timer;
	bool circleDetected = false;
	Rect2d bbox;
	int bboxDim;

	// Create KCF tracker
	Ptr<Tracker> tracker = cv::TrackerCSRT::create();

	VideoCapture src = cv::VideoCapture(0);

	// Checks if webcam wasn't accessed
	if (!src.open(0))
	{
		cout << "Error: Webcam didn't open";
		return 0;
	}

	// Loop through until circle is detected
	for (;;)
	{
		src >> frame;

		if (frame.empty())
			break;

		// Run object detection algorithm (Hough Circle Transform)
		// Convert frame to gray
		cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		// Blur gray frame to reduce background noise
		GaussianBlur(gray, gray, Size(9, 9), 2, 2);

		// Vector that will store x_center, y_center, r
		vector<Vec3f> circles;

		// Apply Hough Circle Transform to frame (will need to closely test radius params)
		// Parameters:
		// 4 - dp, inverse ratio of resolution
		// 5 - minimum distance between detected centers
		// 6 - upper threshold for the internal Canny edge detector
		// 7 - threshold for center detection
		// 8 - minimum radius that will be detected
		// 9 - maximum radius that willl be detected
		HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, gray.rows / 8, 30, 15, 10, 100);

		if (circles.size() > 0)
		{
			circleDetected = true;
			Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
			int radius = cvRound(circles[0][2]);

			bboxDim = 2 * (cvRound(circles[0][2]));
			Rect2d box(cvRound(circles[0][0]) - radius, cvRound(circles[0][1]) - radius, bboxDim, bboxDim);
			bbox = box;

			rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
			imshow("Hough Circle Detection", frame);
			tracker->init(frame, bbox);

			break;
		}

		// Draw detected circle
		/*for (size_t i = 0; i < circles.size(); i++)
		{
			Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
			cout << "Center: " << center << "\n";
			int radius = cvRound(circles[0][2]);
			// Circle center
			cv::circle(frame, center, 3, Scalar(0, 255, 0), -1, 8, 0);
			// Circle outline
			cv::circle(frame, center, radius, Scalar(0, 0, 255), 3, 8, 0);

			// Break for loop after first circle detected
			if (i == 0)
				break;
		}*/
	}

	// Loops indefinitely while tracking object
	while (src.read(frame))
	{
		timer = double(cv::getTickCount());

		bool ok = tracker->update(frame, bbox);

		if (ok)
		{
			cv::rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
		}

		// Calculate frame rate
		float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);

		// Display fps in window
		cv::putText(frame, ("FPS: " + std::to_string(int(fps))), Point(75, 40), cv::FONT_HERSHEY_COMPLEX, 0.7, (20, 230, 20), 2);

		// Display detected circle from camera frame
		namedWindow("Hough Circle Detection", cv::WINDOW_AUTOSIZE);
		imshow("Hough Circle Detection", frame);

		// Press ESC to exit camera 
		if (waitKey(10) == 27)
			break;

	}
	return 0;
}