/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_android_jni_ffmpeg_FfmpegPlayer */

#ifndef _Included_com_android_jni_ffmpeg_FfmpegPlayer
#define _Included_com_android_jni_ffmpeg_FfmpegPlayer
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    init
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_init
  (JNIEnv *, jobject);

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    videoDecode
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_videoDecode
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    videoDecodeDisplay
 * Signature: (Ljava/lang/String;Landroid/view/Surface;)V
 */
JNIEXPORT void JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_videoDecodeDisplay
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    soundDecode
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_soundDecode
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveInit
 * Signature: (IILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveInit
  (JNIEnv *, jobject, jint, jint, jstring);

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveStart
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveStart
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveStop
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveStop
  (JNIEnv *, jobject);

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveClose
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveClose
  (JNIEnv *, jobject);

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveJason
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveJason
  (JNIEnv *, jobject, jstring, jstring);

#ifdef __cplusplus
}
#endif
#endif