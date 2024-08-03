#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#ifdef _WIN32
#pragma comment(lib,"./SDL3-static.lib")
#pragma comment(lib,"imm32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"version.lib")
#pragma comment(lib,"setupapi.lib")
#else
#include <SDL3/SDL_main.h>
#endif