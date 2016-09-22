//
// Created by kang on 9/23/16.
//

#include "jni_lib.h"

#define JNI_AN_MainActivity		"com/kang/demondk/MainActivity"

#define METHOD_NUM 1
JNINativeMethod g_nativeMethod[METHOD_NUM]={
        {"GetStrFromJNI","()Ljava/lang/String;",(void*)GetStrFromJNI}
};

/*
 * 被虚拟机自动调用
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK)
        return JNI_ERR;

    jclass jClass = env->FindClass(JNI_AN_MainActivity);
    env->RegisterNatives(jClass,g_nativeMethod,METHOD_NUM);
    env->DeleteLocalRef(jClass);
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM* vm, void* reserved) {
    JNIEnv *env;
    int nJNIVersionOK = vm->GetEnv((void **)&env, JNI_VERSION_1_6) ;
    jclass jClass = env->FindClass(JNI_AN_MainActivity);
    env->UnregisterNatives(jClass);
    env->DeleteLocalRef(jClass);
}



jstring GetStrFromJNI(JNIEnv *env, jobject callObj) {
    return env->NewStringUTF("String From Jni With c++");
}

