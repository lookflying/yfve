/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class vehicle_CVS_YZ_VehicleTransit_CVS */

#ifndef _Included_vehicle_CVS_YZ_VehicleTransit_CVS
#define _Included_vehicle_CVS_YZ_VehicleTransit_CVS
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_userlogin
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1userlogin
  (JNIEnv *, jclass, jstring, jstring, jstring, jobjectArray);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_userlogout
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1userlogout
  (JNIEnv *, jclass, jstring, jstring);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_gpsupload
 * Signature: (Ljava/lang/String;Lvehicle_CVS/LocationStruct_CVS;)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1gpsupload
  (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_sendvehiclestatus
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1sendvehiclestatus
  (JNIEnv *, jclass, jstring, jint);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_getweather
 * Signature: (Ljava/lang/String;DDILvehicle_CVS/WeatherStruct_DSP;)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1getweather__Ljava_lang_String_2DDILvehicle_1CVS_WeatherStruct_1DSP_2
  (JNIEnv *, jclass, jstring, jdouble, jdouble, jint, jobject);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_getweather
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILvehicle_CVS/WeatherStruct_DSP;)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1getweather__Ljava_lang_String_2Ljava_lang_String_2ILvehicle_1CVS_WeatherStruct_1DSP_2
  (JNIEnv *, jclass, jstring, jstring, jint, jobject);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_gettmc
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1gettmc
  (JNIEnv *, jclass, jstring, jstring, jint);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_sendvehicledata
 * Signature: (Ljava/lang/String;Lvehicle_CVS/VehicleDataStruct_CVS;)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1sendvehicledata
  (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_init
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILvehicle_CVS/VehicleTransitListen_DSP;)V
 */
JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1init__Ljava_lang_String_2Ljava_lang_String_2ILvehicle_1CVS_VehicleTransitListen_1DSP_2
  (JNIEnv *, jclass, jstring, jstring, jint, jobject);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_init
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;Lvehicle_CVS/VehicleTransitListen_DSP;)V
 */
JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1init__Ljava_lang_String_2Ljava_lang_String_2ILjava_lang_String_2ILjava_lang_String_2Lvehicle_1CVS_VehicleTransitListen_1DSP_2
  (JNIEnv *, jclass, jstring, jstring, jint, jstring, jint, jstring, jobject);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_destroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1destroy
  (JNIEnv *, jclass);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_modifyMediaPath
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1modifyMediaPath
  (JNIEnv *, jclass, jstring);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_register
 * Signature: (IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1register
  (JNIEnv *, jclass, jint, jint, jstring, jstring, jstring, jint, jstring);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_getAuthCode
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1getAuthCode
  (JNIEnv *, jclass);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_deregister
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1deregister
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif