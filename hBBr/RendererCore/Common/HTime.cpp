#include "HTime.h"

void HTime::Start()
{
	_start = std::chrono::high_resolution_clock::now();
}

long long HTime::End_ns()
{
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - _start).count();
	return duration;
}

double HTime::End_ms()
{
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - _start).count();
	return (0.000001 * (double)duration);
}

double HTime::End_s()
{
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - _start).count();
	return (0.000000001 * (double)duration);
}

double HTime::FrameRate_ms()
{
	static std::chrono::time_point<std::chrono::steady_clock> start;
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	start = std::chrono::high_resolution_clock::now();
	return (0.000001 * (double)duration);
}
