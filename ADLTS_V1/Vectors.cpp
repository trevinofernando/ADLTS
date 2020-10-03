#include <math.h>
#include "Vectors.h"

const int clockwise = -1, anticlockwise = 1;

int Direction(Vector2 v1, Vector2 v2)
{
	//------------------------ Direction ------------------------------------------
	//
	//  returns positive if v2 is clockwise of this vector,
	//  minus if anticlockwise (Y axis pointing down, X axis to right)
	//------------------------------------------------------------------------
	if (v1.y * v2.x > v1.x* v2.y)
	{
		return anticlockwise;
	}
	else
	{
		return clockwise;
	}
}

float DotProduct2D(Vector2 V1, Vector2 V2)
{
	return V1.x * V2.x + V1.y * V2.y;
}

Vector2 Normalized2D(Vector2 V)
{
	/// Returns a vector in the same direction with a magnitude of 1.
	float magnitude = (float) sqrt((V.x * V.x) + (V.y * V.y));
	if (magnitude == 0)// Check for 0 before division
		return Vector2(0, 0);
	return Vector2(V.x / magnitude, V.y / magnitude);
}

Vector2 ProjectionOf_U_Onto_V(Vector2 U, Vector2 V)
{
	float scalarMultiple = DotProduct2D(U, V) / DotProduct2D(V, V);
	return V * scalarMultiple;
}

Vector2 Clamp(Vector2 U, Vector2 V)
{
	return Vector2((U.x > V.x ? U.x : V.x), (U.y > V.y ? U.y : V.y));
}

float Distance2D(Vector2 U, Vector2 V)
{
	return (float)sqrt((U.x - V.x) * (U.x - V.x) + (U.y - V.y) * (U.y - V.y));
}