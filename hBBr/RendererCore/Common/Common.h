#pragma once

#ifndef HBBR_INLINE
#if __ANDROID__
#define HBBR_INLINE inline
#else
#define HBBR_INLINE __forceinline
#endif
#endif


//#ifdef SHARED_LIBRARY
#ifdef __ANDROID__
	#include <jni.h>
	//#ifdef _USRDLL
	#define HBBR_API __attribute__((visibility("default")))
	//#else
	//#define HBBR_API 
	//#endif
#else
	#if _USE_STATIC
		#define HBBR_API 
	#else
		#ifdef _USE_DLL
			#define HBBR_API __declspec(dllexport)
		#else
			#define HBBR_API __declspec(dllimport)
		#endif
	#endif
#endif
//#else
//#define HBBR_API 
//#endif

#define ENABLE_CODE_OPTIMIZE	__pragma (optimize("", on)) 
#define DISABLE_CODE_OPTIMIZE	__pragma (optimize("", off)) 

#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <functional>
#include <assert.h>
#include "StringTools.h"

//Math include
#include "glm/glm.hpp"
#include "glm/matrix.hpp"
#include "glm/ext.hpp"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
//#include "glm/gtx/rotate_vector.hpp"
//#include "glm/gtc/matrix_transform.hpp"
//#include "glm/gtx/euler_angles.hpp"
//#include "glm/gtx/compatibility.hpp"
//#include "glm/gtx/transform.hpp"

#define _Sleep(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms));

//assert
#if defined(_WIN32)
	#define DE_ASSERT(exp_, msgw_) _ASSERT_EXPR((exp_), (std::string(("\r\n断言: ")) + msgw_ ).c_str());
#elif defined(__ANDROID__)
#elif defined(__linux__)
#endif

#ifndef DE_ASSERT
	#define DE_ASSERT(exp_, msgw_)  ;
#endif

std::string wrapText(const std::string& input, size_t lineWidth);

HBBR_API void MessageOut(std::string msg, bool bExit = false, bool bMessageBox = false ,const char* textColor = ("255,255,255"));

HBBR_API void MsgBox(const char* title, const char* msg);


#ifdef _WIN32

#else
template <typename T, std::size_t N>
constexpr std::size_t countof(const T(&)[N]) {
	return N;
}
#define _countof(x) countof(x)
#endif

#ifndef TEXT
#define TEXT(s) L##s
#endif
