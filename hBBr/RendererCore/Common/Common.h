#pragma once

#ifdef _USRDLL
#define HBBR_API __declspec(dllexport)
#else
#define HBBR_API __declspec(dllimport)
#endif

#include <chrono>
#include <thread>

#define _Sleep(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms));

#if defined(_WIN32)
	#define DE_ASSERT(exp_, msgw_) _ASSERT_EXPR((exp_), HString(HString("\r\nMessage: ")+ HString(msgw_)).c_wstr());
#elif defined(__ANDROID__)
#elif defined(__linux__)
#endif
#ifndef DE_ASSERT
	#define DE_ASSERT ;
#endif

void MessageOut(const char* msg, bool bExit = false, bool bMessageBox = false);