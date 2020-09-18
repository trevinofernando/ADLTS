#include <iostream>
#include <math.h>
#include <chrono>
#include "DetectionSystem.h"
#include <opencv2/tracking/tracker.hpp>

using namespace std;
using namespace cv;

Ptr<Tracker> tracker;
Rect2d bbox;
bool initialized = false;
bool isCircle;
int bboxDim;

bool FindDrone(Vector2 droneCartesianCoord, cv::Mat frame)
{
    if (!initialized)
        Init(frame);

    if (!isCircle)
        return Detect(droneCartesianCoord, frame);
    else
        return Track(droneCartesianCoord, frame);
}

void Init(cv::Mat frame)
{
    cout << "Initializing detection..." << endl;
    initialized = true;
    isCircle = false;

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
}

bool Detect(Vector2 droneCartesianCoord, cv::Mat frame)
{
    Mat gray, blur;
    vector<Vec3f> circles;

    // Start detection timer
    auto start = chrono::high_resolution_clock::now();

    // Convert to grayscale
    cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // Use Gaussian Blur to improve detection
    GaussianBlur(gray, blur, Size(9,9), 2, 2);

    // Apply Hough Circle Transform to frame for circle detection
    // Parameters (in order):
    // 1 - frame to process, 2 - output circle values, 3 - process technique
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
        droneCartesianCoord.x = cvRound(circles[0][0]);
        droneCartesianCoord.y = cvRound(circles[0][1]);
        int radius = cvRound(circles[0][2]);
        cout << "Radius = " << radius << endl;

        bboxDim = 2 * (cvRound(circles[0][2]));
        Rect2d box(cvRound(circles[0][0]) - radius, cvRound(circles[0][1]) - radius, bboxDim, bboxDim);
        bbox = box;

        // Draw circle on image
        Point center(droneCartesianCoord.x, droneCartesianCoord.y);
        circle(frame, center, radius, Scalar(255, 0, 0), 2, 1);

        // Initialize tracker
        tracker->init(frame, bbox);
        circles.clear();
    }

    if (!isCircle)
        return false;
    else
        return true;
}

bool Track(Vector2 droneCartesianCoord, cv::Mat frame)
{
    // Updates tracker with previous bounding box coordinates
    tracker->update(frame,bbox);
    // Draws a rectangle around the new bounding box
    rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);

    // Update central coordinate
    droneCartesianCoord.x = bbox.x + bbox.width/2;
    droneCartesianCoord.y = bbox.y + bbox.height/2;
    // Print central coordinate of bounding box
    cout << "Centroid: (" << droneCartesianCoord.x << ", " << droneCartesianCoord.y << ")" << endl;

    return true;
}
