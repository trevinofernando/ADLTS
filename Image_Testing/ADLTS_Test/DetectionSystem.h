#pragma once

#include "Vectors.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

bool FindDrone(cv::Mat frame, int trackerNum);

void Init(cv::Mat frame, int trackerNum);

bool Detect(cv::Mat frame);

bool Track(cv::Mat frame);
