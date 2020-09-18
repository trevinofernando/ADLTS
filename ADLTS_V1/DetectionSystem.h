#pragma once

#include "Vectors.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

bool FindDrone(Vector2 droneCartesianCoord, cv::Mat frame);

void Init(cv::Mat frame);

bool Detect(Vector2 droneCartesianCoord, cv::Mat frame);

bool Track(Vector2 droneCartesianCoord, cv::Mat frame);
