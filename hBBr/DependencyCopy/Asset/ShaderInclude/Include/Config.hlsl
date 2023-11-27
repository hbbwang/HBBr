#ifndef _CONFIG_HLSL
#define _CONFIG_HLSL

//Platform PC
#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 0
#endif

//Platform Android
#ifndef PLATFORM_ANDROID
#define PLATFORM_ANDROID 0
#endif

//Platform Linux
#ifndef PLATFORM_LINUX
#define PLATFORM_LINUX 0
#endif

//Is using dxt/bc format? Vulkan texture uv need to filp y axis.
#ifndef USE_BC_FORMAT
#define USE_BC_FORMAT 1
#endif

#ifndef MAX_BRIGHTNESS
#define MAX_BRIGHTNESS 16
#endif

#endif