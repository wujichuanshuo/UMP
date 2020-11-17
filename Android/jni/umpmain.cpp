#include <iomanip>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <android/looper.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <jni.h>
#include <unistd.h>

#include "umpserver.h"
#include "umputils.h"


using namespace std;
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    UMPLOGI("JNI_OnLoad");
    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR; // JNI version not supported.
    }
    umpServerStart();
    
    return JNI_VERSION_1_6;
}

#ifdef __cplusplus
}
#endif // __cplusplus