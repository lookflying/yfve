#include <jni.h>
#include "vehicle_CVS_YZ_VehicleTransit_CVS.h"
#include "YzHelper.h"
#include "middleware.h"
#include "message/message.h"
#include <ctime>
#include <stdlib.h>
using namespace std;
/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_userlogin
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1userlogin(
		JNIEnv * env, jclass cls, jstring simcardnum, jstring username,
		jstring password, jobjectArray person) {
	string authCode = jstring2string(env, username);
	string csimcardnum = jstring2string(env, simcardnum);
	set_global_phone_num((MSG_BYTE*)csimcardnum.c_str(), csimcardnum.size());
	Connection &conn = *g_conn_manager.getConnection(0);
	conn.setAuthorizationCode(authCode);
	int ret = conn.connectAndAuthorize();
	return ret;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_userlogout
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1userlogout(
		JNIEnv * env, jclass cls, jstring, jstring) {
	Connection &conn = *g_conn_manager.getConnection(0);
	int ret = conn.disconnect();
	return ret;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_gpsupload
 * Signature: (Ljava/lang/String;Lvehicle_CVS/LocationStruct_CVS;)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1gpsupload(
		JNIEnv * env, jclass cls, jstring simcardnum, jobject cur_loc) {
	string message = locationStruct2string(env, cur_loc);
	msg_body_t *response;
	Connection &conn = *g_conn_manager.getConnection(0);
	int ret = conn.sendMessageAndWait(YZMSGID_POSITION_REPORT, message.c_str(),
			message.length(), &response, false);
	if (ret == 0)
		delete[] (*response).content;
	return ret;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_sendvehiclestatus
 * Signature: (Ljava/lang/String;I)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1sendvehiclestatus(
		JNIEnv * env, jclass cls, jstring, jint) {
	return YZ_OK;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_getweather
 * Signature: (Ljava/lang/String;DDILvehicle_CVS/WeatherStruct_DSP;)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1getweather__Ljava_lang_String_2DDILvehicle_1CVS_WeatherStruct_1DSP_2(
		JNIEnv * env, jclass cls, jstring, jdouble, jdouble, jint, jobject) {
	return YZ_OK;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_getweather
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILvehicle_CVS/WeatherStruct_DSP;)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1getweather__Ljava_lang_String_2Ljava_lang_String_2ILvehicle_1CVS_WeatherStruct_1DSP_2(
		JNIEnv * env, jclass cls, jstring, jstring, jint, jobject) {
	return YZ_OK;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_gettmc
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1gettmc(
		JNIEnv * env, jclass cls, jstring, jstring, jint) {
	return YZ_OK;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_sendvehicledata
 * Signature: (Ljava/lang/String;Lvehicle_CVS/VehicleDataStruct_CVS;)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1sendvehicledata(
		JNIEnv * env, jclass cls, jstring simcardnum, jobject vehicle_data) {
	string message = vehicleDataStruct2string(env, vehicle_data);
	msg_body_t *response;
	Connection &conn = *g_conn_manager.getConnection(0);
	int ret = conn.sendMessageAndWait(YZMSGID_POSITION_REPORT, message.c_str(),
			message.length(), &response, false);
	if (ret == 0)
		delete[] (*response).content;
	return ret;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_init
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILvehicle_CVS/VehicleTransitListen_DSP;)V
 */JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1init__Ljava_lang_String_2Ljava_lang_String_2ILvehicle_1CVS_VehicleTransitListen_1DSP_2(
		JNIEnv *env, jclass cls, jstring terminalId, jstring cvsIp,
		jint cvsPort, jobject vehicleTransitListen) {
	string ip = jstring2string(env, cvsIp);
	initMiddleware(ip, cvsPort);
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_init
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;Lvehicle_CVS/VehicleTransitListen_DSP;)V
 */JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1init__Ljava_lang_String_2Ljava_lang_String_2ILjava_lang_String_2ILjava_lang_String_2Lvehicle_1CVS_VehicleTransitListen_1DSP_2(
		JNIEnv *env, jclass cls, jstring terminalId, jstring cvsIp,
		jint cvsPort, jstring, jint, jstring, jobject) {

}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_destroy
 * Signature: ()V
 */JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1destroy(
		JNIEnv *, jclass) {

}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_3_modifyMediaPath
 * Signature: (Ljava/lang/String;)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_13_1modifyMediaPath(
		JNIEnv * env, jclass cls, jstring) {
	return YZ_OK;
}

/**
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_register
 * Signature: (IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1register(
		JNIEnv *env, jclass cls, jint provinceId, jint cityId, jstring makerId,
		jstring terminalModel, jstring terminalId, jint plateColor,
		jstring plateNum) {
	TerminalRegisterMessage msg;
	msg.provinceId = (MSG_WORD) provinceId;
	msg.cityId = (MSG_WORD) cityId;
	string cmakerId = jstring2string(env, makerId);
	string2bytes(cmakerId, msg.manufactory, sizeof(msg.manufactory));
	string cmodel = jstring2string(env, terminalModel);
	string2bytes(cmodel, msg.terminalModel, sizeof(msg.terminalModel));
	string cterminalId = jstring2string(env, terminalId);
	string2bytes(cterminalId, msg.terminalId, sizeof(msg.terminalId));
	msg.color = (MSG_BYTE) plateColor;
	msg.carPlate = jstring2string(env, plateNum);
	Connection &conn = *g_conn_manager.getConnection(0);

	int ret = conn.connect();
	logcatf("connect return %d\n", ret);
	if (ret != 0) {
		return ret;
	}
	ret = conn.registerTerminal(msg);
	logcatf("register return %d\n", ret);
	return ret;
}
/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_getAuthCode
 * Signature: ()Ljava/lang/String;
 */JNIEXPORT jstring JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1getAuthCode(
		JNIEnv *env, jclass cls) {
	Connection &conn = *g_conn_manager.getConnection(0);
	string cauthCode = conn.authorizationCode();
	jstring authCode = string2jstring(env, cauthCode);
	return authCode;
}
/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_deregister
 * Signature: ()I
 */JNIEXPORT jint JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1deregister(
		JNIEnv * env, jclass cls) {
	Connection &conn = *g_conn_manager.getConnection(0);
	return conn.deregisterTerminal();
}

