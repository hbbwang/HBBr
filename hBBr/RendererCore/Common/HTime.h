#pragma once
#include <chrono>

/* 高精度计时器 */
class HTime
{
public:
	/* 起始时间点 */
	void Start();

	/* 结束时间点,返回经过的时长(纳秒)*/
	long long End_ns();

	/* 结束时间点,返回经过的时长(毫秒)*/
	double End_ms();

	/* 结束时间点,返回经过的时长(秒)*/
	double End_s();

	double FrameRate_ms();

private:

	std::chrono::time_point<std::chrono::steady_clock> _start;

};
