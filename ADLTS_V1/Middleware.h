#pragma once

#include "Vectors.h"
#include <opencv2/opencv.hpp>

extern Vector2 droneCartesianCoord;
extern cv::Mat bgFrame;

void Start();
void FixedUpdate();
void CallNextFrame(std::function<void(void)> func, unsigned int interval);

float FPStoMilliseconds(unsigned int fps);
void RotateTowards(Vector2 targetPosition, float fieldOfView, Vector2 screenSize);

Vector2 ReduceNoise(Vector2 targetPosition, Vector2 prev_targetPosition);

void ResetVelocityMemory();
