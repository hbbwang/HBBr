#pragma once

#ifdef _USRDLL
#define HBBR_API __declspec(dllexport)
#else
#define HBBR_API __declspec(dllimport)
#endif

#include <chrono>
#include <thread>

#define _Sleep(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms));