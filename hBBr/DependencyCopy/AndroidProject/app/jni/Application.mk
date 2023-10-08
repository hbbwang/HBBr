
# Uncomment this if you're using STL in your project
# You can find more information here:
# https://developer.android.com/ndk/guides/cpp-support
# APP_STL := c++_shared

#修改APP_ABI，根据实际需求编译生成对应的arm架构的so库。
#APP_ABI := arm64-v8a armeabi-v7a x86_64 x86 

APP_ABI := arm64-v8a

# Min runtime API level
APP_PLATFORM=android-21

APP_CPPFLAGS += -std=c++17
