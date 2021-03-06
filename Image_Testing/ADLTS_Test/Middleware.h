#pragma once

#include "Vectors.h"

extern Vector2 droneCartesianCoord;

void Start();
void SetInput();
void FixedUpdate();
void CallNextFrame(std::function<void(void)> func, unsigned int interval);

float FPStoMilliseconds(unsigned int fps);
void RotateTowards(Vector2 targetPosition, float fieldOfView, Vector2 screenSize);

Vector2 ReduceNoice(Vector2 targetPosition, Vector2 prev_targetPosition);
