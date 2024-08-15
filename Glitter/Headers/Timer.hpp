#pragma once

#include <GLFW/glfw3.h>

class Timer {
private:
	float cur_time = 0.0f;
	float prev_time = 0.0f;

public:
	inline void Init() { UpdateTime(); }

	inline float GetCurrentTime() { return glfwGetTime(); }
	inline float GetDeltaTime() { return cur_time - prev_time; }
	inline float GetFPS() { return 1.0f / GetDeltaTime(); }
	inline void UpdateTime()
	{
		prev_time = cur_time;
		cur_time = glfwGetTime();
	}
};