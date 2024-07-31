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
#ifdef _USRDLL
#define HBBR_API __declspec(dllexport)
#else
#define HBBR_API __declspec(dllimport)
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
#include <functional>
#include <assert.h>

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

#include "HString.h"

#define _Sleep(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms));

//assert
#if defined(_WIN32)
	#define DE_ASSERT(exp_, msgw_) _ASSERT_EXPR((exp_), (HString(TEXT("\r\n断言: ")) + msgw_ ).c_wstr());
#elif defined(__ANDROID__)
#elif defined(__linux__)
#endif

#ifndef DE_ASSERT
	#define DE_ASSERT(exp_, msgw_)  ;
#endif

HBBR_API void MessageOut(HString msg, bool bExit = false, bool bMessageBox = false ,const char* textColor = ("255,255,255"));

HBBR_API void MsgBox(const char* title, const char* msg);

#if IS_EDITOR
#define HCheck_Color(msg,bbExit,bMessageBox,textColor)\
{\
	HString ggOutMsgggggggg = HString::printf("%s/%s:%d:\n", __FILE__, __func__, __LINE__) + msg;\
	ConsoleDebug::print_endl(ggOutMsgggggggg, textColor);\
	if (bMessageBox && VulkanManager::_bDebugEnable)\
	{\
		const SDL_MessageBoxButtonData buttons[] = {\
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "继续" },\
			{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "中断" },\
		};\
		const SDL_MessageBoxData messageboxdata = {\
			SDL_MESSAGEBOX_WARNING,\
			NULL,\
			"断言",\
			ggOutMsgggggggg.c_str(),\
			SDL_arraysize(buttons),\
			buttons,\
			NULL\
		};\
		int buttonid;\
		SDL_ShowMessageBox(&messageboxdata, &buttonid);\
		if (buttonid == 1)\
		{\
			throw std::runtime_error("Program Exception.");\
		}\
	}\
	if (bbExit)\
	{\
		VulkanApp::AppQuit();\
	}\
}

#define HCheck(Conditions,msg)\
{\
	HString ggOutMsgggggggg = HString::printf("%s/%s:%d:\n", __FILE__, __func__, __LINE__) + msg;\
	ConsoleDebug::print_endl(ggOutMsgggggggg, "255,50,0");\
	if (Conditions)\
	{\
		throw std::runtime_error("Program Exception.");\
	}\
	if (bbExit)\
	{\
		VulkanApp::AppQuit();\
	}\
}
#endif


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
