#include <iostream>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking/tracker.hpp>

using namespace std;
using namespace cv;

VideoCapture cap;
Mat frame;
Ptr<Tracker> tracker;
Rect2d bbox;

void Init();
Point Track();

int main (int argc, char** argv)
{
    Init();

    Track();
 }

void Init()
{
    // Opens camera module
    cap.open(0);
    cap.set(3, 1280);
    cap.set(4, 720);

    if (!cap.isOpened())
    {
        cerr << "ERROR: Unable to open the camera" << endl;
        exit(0);
    }

    string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD", "MEDIANFLOW",
    "GOTURN", "MOSSE", "CSRT"};

    string trackerType = trackerTypes[4];

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
    else if (trackerType == "GOTURN")
        tracker = TrackerGOTURN::create();
    else if (trackerType == "MOSSE")
        tracker = TrackerMOSSE::create();
    else if (trackerType == "CSRT")
        tracker = TrackerCSRT::create();

    // Captures initial frame and gives size of frame size
    cap >> frame;
    //flip(frame, frame, 0);
    cout << "Width: " << frame.size().width << endl;
    cout << "Height: " << frame.size().height << endl;
}

Point Track()
{
    Point centroid;
    Mat gray, blur;
    double timer;
    bool isCircle = false;
    int bboxDim;
    vector<Vec3f> circles;

    VideoWriter video("Demo_Videos/Testing.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, Size(frame.size().width, frame.size().height));

    while(true)
    {
        // Starts timer for FPS count
        timer = double(cv::getTickCount());
        // Captures next frame
        cap >> frame;
        //flip(frame, frame, 0);

        if (frame.empty())
        {
            cerr << "ERROR: Unable to grab from the camera" << endl;
            break;
        }

        // Start detection timer
        auto start = chrono::high_resolution_clock::now();

        while (!isCircle)
        {
            // Convert to grayscale
            cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

            // Use Gaussian Blur to improve detection
            GaussianBlur(gray, blur, Size(9,9), 2, 2);

            // Apply Hough Circle Transform to frame for circle detection
            // Parameters (in order):
            //  1 - frame to process, 2 - output circle values, 3 - process technique
            // 4 - dp, inverse ratio of resolution
            // 5 - minimum distance between detected centers (frame.rows/8)
            // 6 - upper threshold for the internal Canny edge detector
            // 7 - threshold for center detection
            // 8 - minimum radius that will be detected
            // 9 - maximum radius that will be detected
            cv::HoughCircles(blur, circles, cv::HOUGH_GRADIENT, 1, frame.rows/8, 80, 50, 15, 100);

            if (circles.size() > 0)
            {
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> detectTime = (end - start)/1000;
                cout << "Time to detect: " << detectTime.count() << "seconds" << endl;
                isCircle = true;
                Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
                int radius = cvRound(circles[0][2]);
                cout << "Radius = " << radius << endl;

                bboxDim = 2 * (cvRound(circles[0][2]));
                Rect2d box(cvRound(circles[0][0]) - radius, cvRound(circles[0][1]) - radius, bboxDim, bboxDim);
                bbox = box;

                // Draw circle on image
                circle(frame, center, radius, Scalar(255, 0, 0), 2, 1);

                // Initialize tracker
                tracker->init(frame, bbox);
                circles.clear();
            }

            // Calculate frame rate
            float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);
            // Display fps in window
            cv::putText(frame, ("FPS: " + std::to_string(int(fps))), Point(75,40), cv::FONT_HERSHEY_SIMPLEX, 0.7, (57, 255, 20), 2);

            // Outputs frame to window
            imshow("Live", frame);
            waitKey(1);

            // Write frame to output video
            //video.write(frame);

            cap >> frame;
            //flip(frame, frame, 0);
        }


// Updates tracker with previous bounding box coordinates
        if (tracker->update(frame,bbox))
        {
            cout << "Tracking" << endl;
        }
        else
        {
            cout << "Not tracking" << endl;
        }
        // Draws a rectangle around the new bounding box
        rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
        // Print central coordinate of bounding box
        cout << "Centroid: (" << bbox.x + bbox.width/2 << ", " << bbox.y + bbox.height/2 << ")" << endl;

        // Calculate frame rate
        float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);
        // Display fps in window
        cv::putText(frame, ("FPS: " + std::to_string(int(fps))), Point(75,40), cv::FONT_HERSHEY_SIMPLEX, 0.7, (57, 255, 20), 2);

        // Write frame to output video
        //video.write(frame);

        // Outputs frame to window
        imshow("Live", frame);
        waitKey(1);

        // Exits if 'ESC' is pressed
        if (waitKey(1) == 27)
        {
            cap.release();
            video.release();
            return centroid;
        }
    }
}
