// Compatibilidad con el Platform1/Platform2 del APK de Vanth.
// El Java de Vanth declara 5 natives extra que Apollon no exporta; sin ellas el
// juego se cierra con UnsatisfiedLinkError. Aqui van vacias (no hay blur ni texto nativo).
#include <jni.h>

extern "C" {

JNIEXPORT jint JNICALL
Java_com_mojang_minecraftpe_platforms_Platform2_getBlurRadius(JNIEnv *, jclass) {
    return 0;
}

JNIEXPORT jstring JNICALL
Java_com_mojang_minecraftpe_platforms_Platform2_getExtraRects(JNIEnv *env, jclass) {
    return env->NewStringUTF("");
}

JNIEXPORT jstring JNICALL
Java_com_mojang_minecraftpe_platforms_Platform2_pollTextRequest(JNIEnv *, jclass) {
    return nullptr; // null = no hay pedido de texto
}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_platforms_Platform2_setBlurSupported(JNIEnv *, jclass, jboolean) {}

JNIEXPORT void JNICALL
Java_com_mojang_minecraftpe_platforms_Platform2_submitText(JNIEnv *, jclass, jstring, jboolean) {}

} // extern "C"
