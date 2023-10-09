LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

RESOLUTION_PATH := $(LOCAL_PATH)/../../../..

THIRD_PARTY_PATH := $(LOCAL_PATH)/../../../../ThirdParty

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_CFLAGS += \
	-Wall -Wextra \
	-Wmissing-prototypes \
	-Wunreachable-code-break \
	-Wunneeded-internal-declaration \
	-Wmissing-variable-declarations \
	-Wfloat-conversion \
	-Wshift-sign-overflow \
	-Wstrict-prototypes \
	-Wkeyword-macro \

# Warnings we haven't fixed (yet)
LOCAL_CFLAGS += -Wno-unused-parameter -Wno-sign-compare

LOCAL_CXXFLAGS += -std=c++1z

APP_CXXFLAGS += -std=c++1z

#预定义宏
LOCAL_CFLAGS := -DIS_GAME=1 

# 添加库文件路径
LOCAL_LDLIBS += -L$(LOCAL_PATH)/../../libs/$(APP_ABI)/
LOCAL_LDLIBS += -L$(THIRD_PARTY_PATH)/assimp/lib/
LOCAL_LDLIBS += -L$(THIRD_PARTY_PATH)/sdl3/
LOCAL_LDLIBS += -L$(THIRD_PARTY_PATH)/vulkan/

# 添加库
LOCAL_LDLIBS += -lGLESv1_CM 
LOCAL_LDLIBS += -lGLESv2 
LOCAL_LDLIBS += -lOpenSLES 
LOCAL_LDLIBS += -llog 
LOCAL_LDLIBS += -landroid 
LOCAL_LDLIBS += -lc++_shared
#LOCAL_LDLIBS += -lvulkan 
LOCAL_LDLIBS += -lSDL3  -lassimp -lm 

#头文件目录包含
LOCAL_C_INCLUDES += $(LOCAL_PATH)/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/vulkan_wrapper/
LOCAL_C_INCLUDES += $(RESOLUTION_PATH)/RendererCore/
LOCAL_C_INCLUDES += $(RESOLUTION_PATH)/RendererCore/Common/
LOCAL_C_INCLUDES += $(RESOLUTION_PATH)/RendererCore/Core/
LOCAL_C_INCLUDES += $(RESOLUTION_PATH)/RendererCore/Core/Component/
LOCAL_C_INCLUDES += $(RESOLUTION_PATH)/RendererCore/Core/Pass/
LOCAL_C_INCLUDES += $(RESOLUTION_PATH)/RendererCore/Core/Resource/
LOCAL_C_INCLUDES += $(RESOLUTION_PATH)/RendererCore/Editor/
LOCAL_C_INCLUDES += $(RESOLUTION_PATH)/RendererCore/Form/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/assimp/contrib/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/assimp/include/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/crossguid/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/crossguid/Include/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/Imgui/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/Imgui/backends/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/lodepng/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/pugixml/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/sdl3/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/sdl3/Include/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/vulkan/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/vulkan/glm/
LOCAL_C_INCLUDES += $(THIRD_PARTY_PATH)/vulkan/Include/

LOCAL_C_INCLUDES += \
    system/core/libnetutils/include \
    system/core/libutils/include \

#cpp源文件目录包含
LOCAL_SRC_FILES := \
$(subst $(LOCAL_PATH)/,, \
$(wildcard $(LOCAL_PATH)/*.cpp) \
$(wildcard $(LOCAL_PATH)/vulkan_wrapper/*.cpp) \
$(wildcard $(RESOLUTION_PATH)/RendererCore/*.cpp) \
$(wildcard $(RESOLUTION_PATH)/RendererCore/Common/*.cpp) \
$(wildcard $(RESOLUTION_PATH)/RendererCore/Core/*.cpp) \
$(wildcard $(RESOLUTION_PATH)/RendererCore/Core/Component/*.cpp) \
$(wildcard $(RESOLUTION_PATH)/RendererCore/Core/Pass/*.cpp) \
$(wildcard $(RESOLUTION_PATH)/RendererCore/Core/Resource/*.cpp) \
$(wildcard $(RESOLUTION_PATH)/RendererCore/Editor/*.cpp) \
$(wildcard $(RESOLUTION_PATH)/RendererCore/Form/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/assimp/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/assimp/contrib/*.cpp)  \
$(wildcard $(THIRD_PARTY_PATH)/assimp/include/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/crossguid/Include/crossguid/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/Imgui/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/Imgui/backends/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/lodepng/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/pugixml/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/sdl3/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/sdl3/Include/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/vulkan/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/vulkan/glm/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/vulkan/Include/*.cpp) \
$(wildcard $(THIRD_PARTY_PATH)/vulkan/vulkan_wrapper/*.cpp) \
) \

include $(BUILD_SHARED_LIBRARY)
