
#include "Common.h"
#if __ANDROID__
#include "AndroidCommon.h"

std::string _exStoragePath;
//std::string _exAppPath;

std::string GetAndroidExternalStorageDirectory()
{
    if (_exStoragePath.size() <= 2)
    {
        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        jclass clazz = env->FindClass("hBBrManager");
        jmethodID method_id = env->GetStaticMethodID(clazz, "getExternalStoragePath", "()Ljava/lang/String;");
        if (method_id == NULL)
        {
            MessageOut("GetAndroidExternalStorageDirectory :find java function failed.", true, true);
        }
        jstring jstr = (jstring)env->CallStaticObjectMethod(clazz, method_id);
        const char* str = env->GetStringUTFChars(jstr, 0);
        _exStoragePath = (str);
        env->ReleaseStringUTFChars(jstr, str);
        env->DeleteLocalRef(clazz);
    }
    return _exStoragePath;
}
//
//std::string GetAndroidExternalAppDirectory()
//{
//    if (_exAppPath.size() <= 2)
//    {
//        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
//        jobject activity = (jobject)SDL_AndroidGetActivity();
//        jclass clazz(env->GetObjectClass(activity));
//        jmethodID method_id = env->GetStaticMethodID(clazz, "getExternalStoragePath", "()Ljava/lang/String;");
//        if (method_id == NULL)
//        {
//            MessageOut("GetAndroidExternalAppDirectory :find java function failed.", true, true);
//        }
//        jstring jstr = (jstring)env->CallStaticObjectMethod(clazz, method_id);
//        const char* str = env->GetStringUTFChars(jstr, 0);
//        _exAppPath = (str);
//        env->ReleaseStringUTFChars(jstr, str);
//        env->DeleteLocalRef(clazz);
//        env->DeleteLocalRef(activity);
//    }
//    return _exAppPath;
//}

#endif