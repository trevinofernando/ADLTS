#include <iostream>
#include <stdio.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking/tracker.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    Rect2d bbox;
    Point centroid;
    Mat frame, gray, blur;
    double timer;
    bool isCircle = false;
    int bboxDim;

    // Opens camera module
    VideoCapture cap(0);

    if (!cap.isOpened())
    {
        cerr << "ERROR: Unable to open the camera" << endl;
        return 0;
    }

    // Creates object for background subtraction
    Ptr<BackgroundSubtractor> backSub = createBackgroundSubtractorMOG2();

    // CSRT allows high accuracy with low speed (~8fps)
    Ptr<Tracker> tracker = cv::TrackerCSRT::create();
    // KCF provides better speed for slightly worse accuracy (~37fps)
    //Ptr<Tracker> tracker = cv::TrackerKCF::create();
    // MOSSE is used for speed over accuracy (~200fps)
    //Ptr<Tracker> tracker = cv::TrackerMOSSE::create();

    // Captures initial frame and gives size of frame size
    cap >> frame;
    cout << "Width: " << frame.size().width << endl;
    cout << "Height: " << frame.size().height << endl;

    //while (waitKey(1) != 27)
        //imshow("First frame", frame);

    vector<Vec3f> circles;

    VideoWriter video("Demo_Videos/Test_Demo_1.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 15, Size(frame.size().width, frame.size().height));

    while(true)
    {
        // Starts timer for FPS count
        timer = double(cv::getTickCount());
        // Captures next frame
        cap >> frame;

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
            cv::HoughCircles(blur, circles, cv::HOUGH_GRADIENT, 1, frame.rows/8, 80, 50, 15, 25);

            if (circles.size() > 0)
            {
                cout << "Yay, circle detected!!" << endl;
                isCircle = true;
                Point center(cvRound(circles[0][0]), cvRound(circles[0][1]));
                int radius = cvRound(circles[0][2]);
                cout << "Radius = " << radius << endl;

                bboxDim = 2 * (cvRound(circles[0][2]));
                Rect2d box(cvRound(circles[0][0]) - radius, cvRound(circles[0][1]) - radius, bboxDim, bboxDim);
                bbox = box;

                // Draw circle on image
                circle(frame, center, radius, Scalar(255, 0, 0), 2, 1);

                while (waitKey(1) != 27)
                    imshow("Circle Detected", frame);

                // Initialize tracker
                tracker->init(frame, bbox);
            }

            cap >> frame;

            // Convert to grayscale
            cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

            video.write(frame);
        }



        if (frame.empty())
        {
            cerr << "ERROR: Unable to grab from the camera" << endl;
            break;
        }

        // Updates tracker with previous bounding box coordinates
        tracker->update(frame, bbox);
        // Draws a rectangle around the new bounding box
        rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

        // Calculate frame rate
        float fps = cv::getTickFrequency() / (double(cv::getTickCount()) - timer);
        // Display fps in window
        cv::putText(frame, ("FPS: " + std::to_string(int(fps))), Point(75,40), cv::FONT_HERSHEY_COMPLEX, 0.7, (20, 230, 20), 2);

        // Write frame to output video
        video.write(frame);

        // Outputs frame to window
        imshow("Live", frame);

        // Exits if 'ESC' is pressed
        if (waitKey(1) == 27)
            break;
    }

    cap.release();
    return 0;
}
