#pragma once

//More function declarations below struct declarations

struct Vector3
{
	float x = 0, y = 0, z = 0;

	inline Vector3(void) {}
	inline Vector3(const float X, const float Y, const float Z)
	{
		x = X; y = Y; z = Z;
	}

	inline Vector3 operator + (const float C) const
	{
		return Vector3(x + C, y + C, z + C);
	}

	inline Vector3 operator ++ (int)
	{
		x += 1; y += 1; z += 1;
		return *this;
	}

	inline Vector3 operator += (const Vector3& V)
	{
		x += V.x; y += V.y; z += V.z;
		return *this;
	}

	inline Vector3 operator * (const float C) const
	{
		return Vector3(x * C, y * C, z * C);
	}

	inline Vector3 operator / (const float C) const
	{
		return Vector3(x / C, y / C, z / C);
	}

	inline Vector3 operator - (const float C) const
	{
		return Vector3(x - C, y - C, z - C);
	}

	inline Vector3 operator + (const Vector3& V) const
	{
		return Vector3(x + V.x, y + V.y, z + V.z);
	}

	inline Vector3 operator - (const Vector3& V) const
	{
		return Vector3(x - V.x, y - V.y, z - V.z);
	}

	inline Vector3 operator = (const Vector3& V)
	{
		x = V.x; y = V.y; z = V.z;
		return V;
	}

	inline bool operator == (const Vector3& V) const
	{
		return (x == V.x && y == V.y && z == V.z);
	}

	inline bool operator != (const Vector3& V) const
	{
		return (x != V.x || y != V.y || z != V.z);
	}

	inline float Dot(const Vector3& V) const
	{
		return V.x * x + V.y * y + V.z * z;
	}
};

struct Vector2
{
	float x = 0, y = 0;

	inline Vector2(void) {}
	inline Vector2(const float X, const float Y)
	{
		x = X; y = Y;
	}

	inline Vector2 operator + (const float C) const
	{
		return Vector2(x + C, y + C);
	}

	inline Vector2 operator ++ (int)
	{
		x += 1; y += 1;
		return *this;
	}

	inline Vector2 operator += (const Vector2& V)
	{
		x += V.x; y += V.y;
		return *this;
	}

	inline Vector2 operator * (const float C) const
	{
		return Vector2(x * C, y * C);
	}

	inline Vector2 operator / (const float C) const
	{
		return Vector2(x / C, y / C);
	}

	inline Vector2 operator - (const float C) const
	{
		return Vector2(x - C, y - C);
	}

	inline Vector2 operator + (const Vector2& V) const
	{
		return Vector2(x + V.x, y + V.y);
	}

	inline Vector2 operator - (const Vector2& V) const
	{
		return Vector2(x - V.x, y - V.y);
	}

	inline Vector2 operator = (const Vector2& V)
	{
		x = V.x; y = V.y;
		return V;
	}

	inline bool operator == (const Vector2& V) const
	{
		return (x == V.x && y == V.y);
	}

	inline bool operator != (const Vector2& V) const
	{
		return (x != V.x || y != V.y);
	}

	inline float Dot(const Vector2& V) const
	{
		return V.x * x + V.y * y;
	}
};

int Direction(Vector2 v1, Vector2 v2);
float DotProduct2D(Vector2 V1, Vector2 V2);
Vector2 Normalized2D(Vector2 V);
Vector2 ProjectionOf_U_Onto_V(Vector2 U, Vector2 V);
Vector2 Clamp(Vector2 U, Vector2 V);
float Distance2D(Vector2 U, Vector2 V);
