#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <Windows.h>

const unsigned int FPS = 1;

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

	inline float Dot(const Vector3& V) const
	{
		return V.x * x + V.y * y + V.z * z;
	}
};

struct Vector2
{
	float x = 0, y = 0;

	inline Vector2(void) {}
	inline Vector2(const float X, const float Y, const float Z)
	{
		x = X; y = Y;
	}

	inline Vector2 operator + (const float C) const
	{
		return Vector2(x + C, y + C, z + C);
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

	inline float Dot(const Vector2& V) const
	{
		return V.x * x + V.y * y;
	}
};


float FPStoMilliseconds(unsigned int fps);
void timer_start(std::function<void(void)> func, unsigned int interval);
//void Start();
void FixedUpdate();


int main()
{
	
	timer_start(FixedUpdate, FPStoMilliseconds(FPS));
	
	//Prevent the program from ending
	while (!GetAsyncKeyState(VK_RETURN))
	{	
		//Break loop if return key is pressed
	}
}

void timer_start(std::function<void(void)> func, unsigned int interval)
{
	std::thread([func, interval]()
		{
			while (true)
			{
				auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
				func();
				std::this_thread::sleep_until(x);
			}
		}).detach();
}

void FixedUpdate()
{
	std::cout << "I am doing something" << std::endl;
}

float FPStoMilliseconds(unsigned int fps) 
{
	return (float)(1000 / fps);
}
