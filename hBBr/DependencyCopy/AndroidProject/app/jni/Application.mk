
# Uncomment this if you're using STL in your project
# You can find more information here:
# https://developer.android.com/ndk/guides/cpp-support
# APP_STL := c++_shared

#修改APP_ABI，根据实际需求编译生成对应的arm架构的so库。
APP_ABI := armeabi-v7a arm64-v8a x86 x86_64

# Min runtime API level
APP_PLATFORM=android-21

APP_CPPFLAGS += -std=c++11
