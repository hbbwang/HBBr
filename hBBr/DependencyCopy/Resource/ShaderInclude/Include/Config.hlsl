#ifndef _CONFIG_HLSL
#define _CONFIG_HLSL

//Is using dxt/bc format? Vulkan texture uv need to filp y axis.
#ifndef USE_BC_FORMAT
#define USE_BC_FORMAT 1
#endif

#ifndef MAX_BRIGHTNESS
#define MAX_BRIGHTNESS 16
#endif

#endif