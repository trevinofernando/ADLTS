//***NOTE: THIS PROGRAM IS CURRENTLY NOT FUNCTIONAL. IT WILL BE UPDATED IN THE COMING WEEKS.

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
	Mat fgMask, frame;
	double timer;
	bool circleDetected = false;
	Rect2d bbox;
	int bboxDim;

	Ptr<BackgroundSubtractor> backSub = createBackgroundSubtractorMOG2();
	Ptr<Tracker> tracker = cv::TrackerCSRT::create();

	VideoCapture src = cv::VideoCapture(0);

	// Checks if webcam wasn't accessed
	if (!src.open(0))
	{
		cout << "Error: Webcam didn't open";
		return 0;
	}

	// Streams video until ESC is pressed
	while (true)
	{
		timer = double(cv::getTickCount());

		src >> frame;

		if (frame.empty())
			break;

		// Use background subtraction on first frame
		backSub->apply(frame, fgMask);

		// Vector that will store x_center, y_center, r
		vector<Vec3f> circles;

		if (!circleDetected)
		{
			// Apply Hough Circle Transform to frame (will need to closely test radius params)
			// Parameters:
			// 4 - dp, inverse ratio of resolution
			// 5 - minimum distance between detected centers
			// 6 - upper threshold for the internal Canny edge detector
			// 7 - threshold for center detection
			// 8 - minimum radius that will be detected
			// 9 - maximum radius that willl be detected
			HoughCircles(frame, circles, cv::HOUGH_GRADIENT, 1, frame.rows / 8, 30, 15, 10, 50);

			if (circles.size() > 0)
			{
				circleDetected = true;
				Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
				int radius = cvRound(circles[0][2]);

				bboxDim = 2 * (cvRound(circles[0][2]));
				Rect2d box(cvRound(circles[0][0]) - radius, cvRound(circles[0][1]) - radius, bboxDim, bboxDim);
				bbox = box;
			}
		}
		else
		{
			bool ok = tracker->update(frame, bbox);

			if (ok)
			{
				cv::rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
			}
		}

		// Calculate frame rate
		float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);

		// Display fps in window
		cv::putText(frame, ("FPS: " + std::to_string(int(fps))), Point(75, 40), cv::FONT_HERSHEY_COMPLEX, 0.7, (20, 230, 20), 2);

		imshow("Original", fgMask);
		imshow("Foreground", frame);

		if (waitKey(10) == 27)
			break;
	}

	return 0;
}