LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

#SDL_PATH := ../SDL

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Include/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SDL_PATH)/include

# Add your application source files here...
LOCAL_SRC_FILES := main.cpp

#LOCAL_SHARED_LIBRARIES := SDL3

# 添加库文件路径
LOCAL_LDLIBS += -L$(LOCAL_PATH)/../../libs/$(APP_ABI)/
# 添加库
LOCAL_LDLIBS += -lGLESv1_CM 
LOCAL_LDLIBS += -lGLESv2 
LOCAL_LDLIBS += -lOpenSLES 
LOCAL_LDLIBS += -llog 
LOCAL_LDLIBS += -landroid 
LOCAL_LDLIBS += -lc++_shared
LOCAL_LDLIBS += -lm 
LOCAL_LDLIBS += -lSDL3
LOCAL_LDLIBS += -lRendererCore_Android 
LOCAL_LDLIBS += -lvulkan 
LOCAL_LDLIBS += -lassimp 

include $(BUILD_SHARED_LIBRARY)
