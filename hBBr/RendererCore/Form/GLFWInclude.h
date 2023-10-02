#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#ifdef _WIN32
	#include <SDL3/SDL_syswm.h>
#endif

#pragma comment(lib,"SDL3-static.lib")

#ifdef _WIN32
#pragma comment(lib,"imm32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"version.lib")
#pragma comment(lib,"setupapi.lib")
#endif