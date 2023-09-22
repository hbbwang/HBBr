#include "HTime.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

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

HString HTime::CurrentDateAndTime()
{
	// 获取当前时间点
	auto now = std::chrono::system_clock::now();

	// 将当前时间点转换为 std::time_t 类型
	std::time_t current_time = std::chrono::system_clock::to_time_t(now);

	// 使用 std::localtime 将 std::time_t 转换为 std::tm 结构
	std::tm local_time;
	//local_time = std::localtime(&current_time);
	localtime_s(&local_time, &current_time);

	// 创建一个输出字符串流
	std::ostringstream oss;

	// 使用 std::put_time 将日期和时间格式化到输出字符串流中
	oss << std::put_time(&local_time, "%Y-%m-%d-%H-%M-%S");

	// 从输出字符串流中获取 std::string
	std::string datetime_str = oss.str();

	return datetime_str.c_str();
}

HString HTime::CurrentDateAndTimeH(bool setW)
{
	// 获取当前时间点
	auto now = std::chrono::system_clock::now();

	// 获取自纪元以来的毫秒数和微秒数
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000;

	// 将当前时间点转换为 std::time_t 类型
	std::time_t current_time = std::chrono::system_clock::to_time_t(now);

	// 使用 std::localtime 将 std::time_t 转换为 std::tm 结构
	std::tm local_time;
	//local_time = std::localtime(&current_time);
	localtime_s(&local_time, &current_time);

	// 创建一个输出字符串流
	std::ostringstream oss;

	// 使用 std::put_time 将日期和时间格式化到输出字符串流中
	if(setW)
		oss << std::put_time(&local_time, "%Y-%m-%d-%H-%M-%S") << '-' << std::setw(3) << ms.count() << '-' << std::setw(3) << us.count();
	else
		oss << std::put_time(&local_time, "%Y-%m-%d-%H-%M-%S") << '-' << ms.count() << '-' << us.count();

	// 从输出字符串流中获取 std::string
	std::string datetime_str = oss.str();

	return datetime_str.c_str();
}