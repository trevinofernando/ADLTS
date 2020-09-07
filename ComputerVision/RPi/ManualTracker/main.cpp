#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking/tracker.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    Rect2d roi;
    Point centroid;
    Mat frame;
    double timer;

    // Opens camera module
    VideoCapture cap(0);

    if (!cap.isOpened())
    {
        cerr << "ERROR: Unable to open the camera" << endl;
        return 0;
    }

    // CSRT allows high accuracy with low speed (~4fps)
    //Ptr<Tracker> tracker = cv::TrackerCSRT::create();
    // KCF provides better speed for slightly worse accuracy (~12fps)
    //Ptr<Tracker> tracker = cv::TrackerKCF::create();
    // MOSSE is used for speed over accuracy (~33fps)
    Ptr<Tracker> tracker = cv::TrackerMOSSE::create();

    // Captures initial frame and gives size of frame size
    cap >> frame;
    cout << "Width: " << frame.size().width << endl;
    cout << "Height: " << frame.size().height << endl;

    // Allows user to create custom bounding box for tracking
    roi = selectROI("tracker", frame);
    // Calculates central cordinate pair of bounding box and prints the values
    centroid = Point((roi.x + roi.width)/2, (roi.y + roi.height)/2);
    cout << "Center is at " << centroid.x << ", " << centroid.y << endl;\
    // Initializes the tracker with the bounding box
    tracker->init(frame, roi);

    while(true)
    {
        // Starts timer for FPS count
        timer = double(cv::getTickCount());
        // Captures next frame
        cap >> frame;

        if (frame.empty())
        {
            cerr << "ERROR: Unable to grab from the camera" << endl;
            break;
        }
        // Updates tracker with previous bounding box coordinates
        tracker->update(frame, roi);
        // Draws a rectangle around the new bounding box
        rectangle(frame, roi, Scalar(255, 0, 0), 2, 1);

        // Calculates the FPS of the video stream and prints it on the video frame
        float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);
        cv::putText(frame, ("FPS: " + std::to_string(int(fps))), Point(75, 40), cv::FONT_HERSHEY_COMPLEX, 0.7, (20, 230, 20), 2);
        // Outputs frame to window
        imshow("Live", frame);
        // Exits if 'ESC' is pressed
        if (waitKey(1) == 27)
            break;
    }

    return 0;
}
