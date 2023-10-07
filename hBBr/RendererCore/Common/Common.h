#pragma once

//#ifdef SHARED_LIBRARY

#ifdef _USRDLL
#define HBBR_API __declspec(dllexport)
#else
#define HBBR_API __declspec(dllimport)
#endif

//#else
//#define HBBR_API 
//#endif

#define ENABLE_CODE_OPTIMIZE	__pragma (optimize("", on)) 
#define DISABLE_CODE_OPTIMIZE	__pragma (optimize("", off)) 

#include <chrono>
#include <thread>
#include <assert.h>

//Math include
#include "glm/glm.hpp"
#include "glm/gtx/compatibility.hpp"
#include "glm/matrix.hpp"
#include "glm/ext.hpp"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include "HString.h"

#define _Sleep(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms));

#if defined(_WIN32)
	#define DE_ASSERT(exp_, msgw_) _ASSERT_EXPR((exp_), HString(HString("\r\nMessage: ")+ HString(msgw_)).c_wstr());
#elif defined(__ANDROID__)
	
#elif defined(__linux__)
#endif
#ifndef DE_ASSERT
	#define DE_ASSERT(exp_, msgw_)  ;
#endif

void MessageOut(const char* msg, bool bExit = false, bool bMessageBox = false ,const char* textColor = ("255,255,255"));