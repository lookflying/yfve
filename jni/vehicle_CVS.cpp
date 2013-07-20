#include <jni.h>
#include "vehicle_CVS_YZ_VehicleTransit_CVS.h"
#include "YzHelper.h"
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
	MSG_DWORD upload_alarm_status = 0;
	MSG_DWORD upload_status = 0;
	MSG_BCD upload_time[6];
	MSG_DWORD upload_longitude;
	MSG_DWORD upload_latitude;
	MSG_WORD upload_altitude;
	MSG_WORD upload_speed;
	MSG_WORD upload_direction;

	jclass loc_cls = env->GetObjectClass(cur_loc);
	//get alarm status
	jfieldID alarm_status_id = env->GetFieldID(loc_cls, "alarmstatus", "I");
	jint alarm_status = env->GetIntField(cur_loc, alarm_status_id);
	switch (alarm_status) {
	case 0: //正常
		MSG_SET_DWORD(upload_alarm_status, 0x00, 0x00, 0x00, 0x00);
		break;
	case 1: //紧急报警
		MSG_SET_DWORD(upload_alarm_status, 0x00, 0x00, 0x00, 0x01);
		break;
	case 2: //终端主电源掉电
		MSG_SET_DWORD(upload_alarm_status, 0x00, 0x00, 0x01, 0x00);
		break;
	case 3: //超速报警
		MSG_SET_DWORD(upload_alarm_status, 0x00, 0x00, 0x00, 0x02);
		break;
	default:
		break;
	}
	//status default set to 0
	upload_status = 0;
	//get longitude
	jfieldID longitude_id = env->GetFieldID(loc_cls, "longitude", "D");
	jdouble longitude = env->GetDoubleField(cur_loc, longitude_id);
	upload_longitude = static_cast<MSG_DWORD>(longitude * 1000000);
	//get latitude
	jfieldID latitude_id = env->GetFieldID(loc_cls, "latitude", "D");
	jdouble latitude = env->GetDoubleField(cur_loc, latitude_id);
	upload_latitude = static_cast<MSG_DWORD>(latitude * 1000000);
	//get altitude
	jfieldID altitude_id = env->GetFieldID(loc_cls, "altitude", "F");
	jfloat altitude = env->GetFloatField(cur_loc, altitude_id);
	upload_altitude = static_cast<MSG_WORD>(altitude);
	//get speed
	jfieldID speed_id = env->GetFieldID(loc_cls, "speed", "F");
	jfloat speed = env->GetFloatField(loc_cls, speed_id);
	upload_speed = static_cast<MSG_WORD>(speed);
	//get direction
	jfieldID direction_id = env->GetFieldID(loc_cls, "direction", "F");
	jfloat direction = env->GetFloatField(cls, direction_id);
	upload_direction = static_cast<MSG_WORD>(direction);
	//get time
	jfieldID time_id = env->GetFieldID(loc_cls, "time", "J");
	jlong time = env->GetLongField(cur_loc, time_id);
	time_t t = static_cast<time_t>(time / 1000);
	struct tm * ptm;
	setenv("TZ", "Asia/Shanghai", 1);
	tzset();
	ptm = localtime(&t);
	unsigned char byte0, byte1;
	int year = ptm->tm_year % 100;
	int month = ptm->tm_mon + 1;
	int day = ptm->tm_mday;
	int hour = ptm->tm_hour;
	int minute = ptm->tm_min;
	int sec = ptm->tm_sec;
	byte0 = (MSG_BYTE) ((year / 10) & 0xff);
	byte1 = (MSG_BYTE) ((year % 10) & 0xff);
	MSG_BYTE2BCD(byte0, byte1, upload_time[0]);
	byte0 = (MSG_BYTE) ((month / 10) & 0xff);
	byte1 = (MSG_BYTE) ((month % 10) & 0xff);
	MSG_BYTE2BCD(byte0, byte1, upload_time[1]);
	byte0 = (MSG_BYTE) ((day / 10) & 0xff);
	byte1 = (MSG_BYTE) ((day % 10) & 0xff);
	MSG_BYTE2BCD(byte0, byte1, upload_time[2]);
	byte0 = (MSG_BYTE) ((hour / 10) & 0xff);
	byte1 = (MSG_BYTE) ((hour % 10) & 0xff);
	MSG_BYTE2BCD(byte0, byte1, upload_time[3]);
	byte0 = (MSG_BYTE) ((minute / 10) & 0xff);
	byte1 = (MSG_BYTE) ((minute % 10) & 0xff);
	MSG_BYTE2BCD(byte0, byte1, upload_time[4]);
	byte0 = (MSG_BYTE) ((sec / 10) & 0xff);
	byte1 = (MSG_BYTE) ((sec % 10) & 0xff);
	MSG_BYTE2BCD(byte0, byte1, upload_time[5]);
	string message = "";
	message += big_endian(upload_alarm_status);
	message += big_endian(upload_status);
	message += big_endian(upload_longitude);
	message += big_endian(upload_latitude);
	message += big_endian(upload_altitude);
	message += big_endian(upload_speed);
	message += big_endian(upload_direction);
	message.append((char*) &upload_time[0], 6);
	msg_body_t *response;
	Connection &conn = *g_conn_manager.getConnection(0);
	int ret = conn.sendMessageAndWait(YZMSGID_POSITION_REPORT, message.c_str(),
			message.length(), &response, false);

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
		JNIEnv * env, jclass cls, jstring, jobject) {
	return YZ_OK;
}

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_init
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILvehicle_CVS/VehicleTransitListen_DSP;)V
 */JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1init__Ljava_lang_String_2Ljava_lang_String_2ILvehicle_1CVS_VehicleTransitListen_1DSP_2(
		JNIEnv * env, jclass cls, jstring, jstring, jint, jobject);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_init
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;Lvehicle_CVS/VehicleTransitListen_DSP;)V
 */JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1init__Ljava_lang_String_2Ljava_lang_String_2ILjava_lang_String_2ILjava_lang_String_2Lvehicle_1CVS_VehicleTransitListen_1DSP_2(
		JNIEnv * env, jclass cls, jstring, jstring, jint, jstring, jint,
		jstring, jobject);

/*
 * Class:     vehicle_CVS_YZ_VehicleTransit_CVS
 * Method:    yz_2_destroy
 * Signature: ()V
 */JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1destroy(
		JNIEnv *, jclass);

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
	int ret = conn.registerTerminal(msg);
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
 * Signature: ()V
 */JNIEXPORT void JNICALL Java_vehicle_1CVS_YZ_1VehicleTransit_1CVS_yz_12_1deregister(
		JNIEnv *, jclass) {
	Connection &conn = *g_conn_manager.getConnection(0);
	(void) conn.deregisterTerminal();
}

