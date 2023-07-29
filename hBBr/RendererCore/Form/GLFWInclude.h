#pragma once

#pragma comment(lib,"glfw3.lib")

#if defined(_WIN32)
	#define GLFW_EXPOSE_NATIVE_WIN32	1	//GLFW support win32
	#elif defined(__linux__)
#endif

#define GLFW_INCLUDE_VULKAN 1

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"