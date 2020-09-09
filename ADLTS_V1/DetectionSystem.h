#pragma once

#include "Vectors.h"

bool FindDrone(Vector2 droneCartesianCoord);

void Init();

bool Detect(Vector2 droneCartesianCoord);

bool Track(Vector2 droneCartesianCoord);
