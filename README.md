# DemoJNI

一个关于Android Studio 2.2 版本下NDK开发的Demo

说明：http://www.jianshu.com/p/3a7609fc5141


#前言
期待了几个月的Android Studio 2.2 版本稳定版昨天终于发布，迫不及待的更新尝试。这次[更新内容]颇多。我主要关注NDK开发，所以期待这一版本带来的c/c++支持的增强。本文介绍一下这两天折腾新版AS开发和调试NDK的一些经验

![Paste_Image.png](http://upload-images.jianshu.io/upload_images/1908868-dc108354f2eb160c.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


# 一、NDK的支持

>Android Studio 2.2 的NDK开发支持 Cmake和ndk-build两种方式。相比与以前的gradle去配置ndk编译目录什么的简直是方便多了。对于老的通过Android.mk文件编译的NDK项目，直接一条配置整个项目就可以被AS支持了。

##1.Cmake方式使用AS开发调试NDK
> 新版的Android Studio支持使用Cmake构建c/c++工程。相比与上一版要通过gradle来配置c/c++工程简单了多，而且也便于老项目的迁移了，下面先来一个简单的例子

###1).新建一个空的工程
默认建一个空的工程，只包含一个MainActivity
###2).创建jni目录和cpp文件
在左侧Project栏选择app，右键-->New-->Folder-->JNI Folder ，app目录下会多出一个cpp（貌似上一版还是jni目录）的目录。

![Paste_Image.png](http://upload-images.jianshu.io/upload_images/1908868-c524592a91b7c04b.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

在cpp目录右键New-->C/C++ Source File ，新建两个文件jni_lib.cpp/h

![Paste_Image.png](http://upload-images.jianshu.io/upload_images/1908868-66fc55156946c66c.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

###3).配置jni工程
进入文件管理器，在jni目录下创建CmakeLists.txt文件
```
# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib
             # 这个是jni编译生产的so库的名字
             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             # 要编译的c/c++文件列表 文件路径想对于cmake文件路径
             ./jni_lib.cpp )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib
              # 依赖的系统so库
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
```
Project栏app右键，Link C++ Project With Gradle 选择CMakeLists.txt文件。或者也可以直接在gradle中加入如下配置即可
```
externalNativeBuild{    
      cmake{
        path file("src/main/jni/CMakeLists.txt") 
   }
}
```
选择Gralde同步之后就完成了jni工程的创建
###4).写第一个jni函数

我们可以完成我们的第一个jni函数
jni_lib.h
```
#ifndef DEMOCMAKE_JNI_LIB_H
#define DEMOCMAKE_JNI_LIB_H
#include "jni.h"
jstring GetStrFromJNI(JNIEnv* env,jobject callObj);
#endif //DEMOCMAKE_JNI_LIB_H
```
jni_lib.cpp
```
#include "jni_lib.h"
jstring GetStrFromJNI(JNIEnv *env, jobject callObj)
 {    
      return env->NewStringUTF("String From Jni With c++");
}
```
光有这些是不够的，java虚拟机是无法直接找到GetStrFromJNI这个函数的，需要通过调用JNI_OnLoad函数，实现JNI函数和java native声明的对接。关于[jni的知识]可以多Google一下学习。

so库的加载和native函数的声明
MainActivity.java
```
public class MainActivity extends AppCompatActivity {

    static {
        /*
        加载动态库，动态库加载的时候 JNI_OnLoad函数会被调用
        
        在JNI——OnLoad函数中，Java虚拟机通过函数表的形式将JNI函数和java类中native函数对应起来
         */
        System.loadLibrary("native-lib");
    }
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView textView= (TextView) findViewById(R.id.text);
        textView.setText(GetStrFromJNI());
    }
    
    /*
    Jni 函数的声明
    当调用到此函数时，java虚拟机会通过JNI_OnLoad里注册的函数表找到对应的函数去执行
     */
    private native String GetStrFromJNI();
}
```

JNI_OnLoad的实现
jni_lib.cpp
```
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


```

##2.ndk-build方式使用AS开发调试NDK
> ndk-build方式与cmake方式类似，只需要将cmake文件改写为Android.mk和Appliction.mk文件。在CMakeLists.txt加载的位置将CMakeLists.txt替换为Android.mk即可

Android.mk
```

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
APP_ABI := all

LOCAL_MODULE    := native-lib
LOCAL_CPPFLAGS 	:= -O0 -D_UNICODE -DUNICODE -DUSE_DUMP -Wno-error=format-security
LOCAL_CPP_EXTENSION := .cpp
LOCAL_LDLIBS    := -lm -llog -lz
LOCAL_SHORT_COMMANDS := true
INC_DIRS = -I$(LOCAL_PATH)/jni
LOCAL_CPPFLAGS += $(INC_DIRS)

LOCAL_SRC_FILES	:= \
	jni_lib.cpp	\	\


LOCAL_SHARED_LIBRARIES += libandroid_runtime
	

include $(BUILD_SHARED_LIBRARY)

```
Appliction.mk
```
APP_ABI := all
NDK_TOOLCHAIN_VERSION := clang
APP_SHORT_COMMANDS      := true
APP_STL := stlport_static
APP_CPPFLAGS := -std=gnu++11 -D__STDC_LIMIT_MACROS

```
在gradle的配置中cmake的配置替换为
```
//    externalNativeBuild{
//        cmake{
//            path file("src/main/jni/CMakeLists.txt")
//        }
//    }
    externalNativeBuild{
        ndkBuild{
            path file("src/main/jni/Android.mk")
        }
    }
```

##4.一些简单的配置
[产见]
# 二、体验改进和存在的问题
## 1.编译
>配置好CmakeLists.txt或者Android.mk之后，编译的时候，Android Stuido 会自动进行NDK的编译。我测试了mac和ubuntu都没有问题，在windows下似乎有些问题。发现，当Android.mk项目中有依赖的静态库的时候，编译处理有些问题，无法通过编译。windows党自行测试。

## 2.调试
>我升级新版Androd Studio的直接原因就是，终于可以方便的调试底层代码了。但是经过两天的使用还是发现有些问题的。

* 调试带有NDK项目的工程的时候，Android Studio会同时启动两个调试器，一个针对NDK的lldb调试器，和Java调试器。
* 默认NDK的调试是开的，因为打开NDK调试，启动调试的时候还是挺慢的，不需要的时候，可以将工程设置里的Debuger从Auto 改为Java
* 另为需要注意的问题是，如果NDK代码为了项目的Model中，必须在如下位置Symbol Directories，增加NDK所在项目的根路径，否则LLDB调试器会报找不到符号文件错误，是无法进行调试的

![Paste_Image.png](http://upload-images.jianshu.io/upload_images/1908868-f7b941c77c6acde9.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

* 在Ubuntu测试的时候还发现一个见鬼的问题，有时候配置好一个项目，程序死活不会停在c/c++的断点出。刚开始感觉很莫名其妙，配置两个一模一样的工程，一个可以调试NDK，一个不可以。最后经过反复测试，才发现一个问题：**当NDK代码位于Model中的时候，这个项目的APP的目录深度一定不能大于NDK所在Model的路径深度！**

# 后话
> Android Studio 2.2的发布，尽管还有许多问题，在这两天的使用中还是发现有很多不稳定的地方。在编辑和调试大的项目时还是经常容易出现异常。但是毕竟对于Androd Studio 进行NDK开发来说还是很大的进步。本人才疏学浅，近一年一直学习和使用NDK，发现网上资料甚少，写次博文全当交流学习。欢迎有共同爱好和需求的朋友交流讨论，共同学习

本文[Sample代码]



[更新内容]:http://www.oschina.net/news/77286/android-studio-2-2-stable?_t=t
[jni的知识]:http://blog.csdn.net/dadoneo/article/details/7597900
[产见]:https://developer.android.com/studio/projects/add-native-code.html
[Sample代码]:https://github.com/Yufeikang/DemoJNI
