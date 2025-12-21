#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_system.h>
#include <iostream>

#if __ANDROID__
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
extern std::string _exStoragePath;
//extern std::string _exAppPath;

//获取安卓手机根目录(beta)
std::string GetAndroidExternalStorageDirectory();
//std::string GetAndroidExternalAppDirectory();
#endif