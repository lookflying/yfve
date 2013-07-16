/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class YzJni */

#ifndef _Included_YzJni
#define _Included_YzJni
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     YzJni
 * Method:    yz_2_userlogin
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_12_1userlogin
  (JNIEnv *, jclass, jstring, jstring, jstring, jobjectArray);

/*
 * Class:     YzJni
 * Method:    yz_2_userlogout
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_12_1userlogout
  (JNIEnv *, jclass, jstring, jstring);

/*
 * Class:     YzJni
 * Method:    yz_3_gpsupload
 * Signature: (Ljava/lang/String;LLocationSturt_CVS;)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_13_1gpsupload
  (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     YzJni
 * Method:    yz_2_sendvehiclestatus
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_12_1sendvehiclestatus
  (JNIEnv *, jclass, jstring, jint);

/*
 * Class:     YzJni
 * Method:    yz_3_getweather
 * Signature: (Ljava/lang/String;DDILWeatherSturt_DSP;)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_13_1getweather__Ljava_lang_String_2DDILWeatherSturt_1DSP_2
  (JNIEnv *, jclass, jstring, jdouble, jdouble, jint, jobject);

/*
 * Class:     YzJni
 * Method:    yz_3_weathercallback
 * Signature: (LWeatherSturt_DSP;)V
 */
JNIEXPORT void JNICALL Java_YzJni_yz_13_1weathercallback
  (JNIEnv *, jclass, jobject);

/*
 * Class:     YzJni
 * Method:    yz_3_getweather
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILWeatherSturt_DSP;)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_13_1getweather__Ljava_lang_String_2Ljava_lang_String_2ILWeatherSturt_1DSP_2
  (JNIEnv *, jclass, jstring, jstring, jint, jobject);

/*
 * Class:     YzJni
 * Method:    yz_3_remotedescallback
 * Signature: (ILjava/util/List;I)V
 */
JNIEXPORT void JNICALL Java_YzJni_yz_13_1remotedescallback
  (JNIEnv *, jclass, jint, jobject, jint);

/*
 * Class:     YzJni
 * Method:    yz_3_gettmc
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_13_1gettmc
  (JNIEnv *, jclass, jstring, jstring, jint);

/*
 * Class:     YzJni
 * Method:    yz_3_TMCcallback
 * Signature: (LTMCStruct_DSP;)V
 */
JNIEXPORT void JNICALL Java_YzJni_yz_13_1TMCcallback
  (JNIEnv *, jclass, jobject);

/*
 * Class:     YzJni
 * Method:    yz_3_sendvehicledata
 * Signature: (Ljava/lang/String;LVehicleDataStruct_CVS;)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_13_1sendvehicledata
  (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     YzJni
 * Method:    yz_2_init
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_YzJni_yz_12_1init__Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv *, jclass, jstring, jstring, jint);

/*
 * Class:     YzJni
 * Method:    yz_2_init
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;LVehicleTransitListen_DSP;)V
 */
JNIEXPORT void JNICALL Java_YzJni_yz_12_1init__Ljava_lang_String_2Ljava_lang_String_2ILjava_lang_String_2ILjava_lang_String_2LVehicleTransitListen_1DSP_2
  (JNIEnv *, jclass, jstring, jstring, jint, jstring, jint, jstring, jobject);

/*
 * Class:     YzJni
 * Method:    yz_2_destory
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_YzJni_yz_12_1destory
  (JNIEnv *, jclass);

/*
 * Class:     YzJni
 * Method:    yz_2_destroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_YzJni_yz_12_1destroy
  (JNIEnv *, jclass);

/*
 * Class:     YzJni
 * Method:    yz_3_loginstatecallback
 * Signature: (Ljava/lang/String;JLjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_YzJni_yz_13_1loginstatecallback
  (JNIEnv *, jclass, jstring, jlong, jstring, jint);

/*
 * Class:     YzJni
 * Method:    yz_3_modifyMediaPath
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_YzJni_yz_13_1modifyMediaPath
  (JNIEnv *, jclass, jstring);

#ifdef __cplusplus
}
#endif
#endif
