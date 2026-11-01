#include <jni.h>
extern "C" {
JNIEXPORT jint JNICALL Java_com_mojang_minecraftpe_platforms_Platform2_getBlurRadius(JNIEnv *, jclass) { return 0; }
JNIEXPORT jstring JNICALL Java_com_mojang_minecraftpe_platforms_Platform2_getExtraRects(JNIEnv *env, jclass) { return env->NewStringUTF(""); }
JNIEXPORT jstring JNICALL Java_com_mojang_minecraftpe_platforms_Platform2_pollTextRequest(JNIEnv *, jclass) { return nullptr; }
JNIEXPORT void JNICALL Java_com_mojang_minecraftpe_platforms_Platform2_setBlurSupported(JNIEnv *, jclass, jboolean) {}
JNIEXPORT void JNICALL Java_com_mojang_minecraftpe_platforms_Platform2_submitText(JNIEnv *, jclass, jstring, jboolean) {}
}
