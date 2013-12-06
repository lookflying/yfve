#include "YzHelper.h"
#include "Middleware.h"
#include <unistd.h>
#include <cstring>
#include <android/log.h>
#define EV_STANDALONE 1
#include "ev.h"
using namespace std;

/**
 * logcat buffer
 */
char YzHelper::logcat_buf[1024];

/**
 * printf to logcat
 */
void YzHelper::logcatf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsnprintf(logcat_buf, sizeof(logcat_buf), fmt, args);
	va_end(args);
	__android_log_write(ANDROID_LOG_DEBUG, "logcatf", logcat_buf);
}

void YzHelper::logcat_hex(const char* buf, unsigned int len) {
	int pos = 0;
	for (unsigned int i = 0; i < len; ++i) {
		snprintf(logcat_buf + 3 * i, sizeof(logcat_buf) - 3 * i, "%02x ",
				buf[i]);
	}
	__android_log_write(ANDROID_LOG_DEBUG, "logcatf", logcat_buf);

}

/**
 * jstring to string
 */
string YzHelper::jstring2string(JNIEnv* env, jstring jstr) {
	const char *cptr = env->GetStringUTFChars(jstr, 0);
	jsize len = env->GetStringUTFLength(jstr);
	string str = "";
	str.append(cptr, len);
	env->ReleaseStringUTFChars(jstr, cptr);
	return str;
}

/**
 * string to jstring
 */
jstring YzHelper::string2jstring(JNIEnv* env, const string str) {
	jclass strClass = env->FindClass("java/lang/String");
	jmethodID ctorID = env->GetMethodID(strClass, "<init>",
			"([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(str.size());
	env->SetByteArrayRegion(bytes, 0, str.size(), (jbyte*) &str.c_str()[0]);
	jstring encoding = env->NewStringUTF("utf-8");
	return (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
}

msg_body_t YzHelper::string2msg_body(std::string content) {
	msg_body_t body;
	body.content = (MSG_BYTE*) &content.c_str()[0];
	body.length = content.size();
	return body;
}
void YzHelper::string2bytes(const std::string str, MSG_BYTE* target, unsigned int len) {
	if (str.size() >= len) {
		memcpy(target, str.c_str(), len);
	} else {
		memcpy(target, str.c_str(), str.size());
		memset(target + str.size(), 0, len - str.size());
	}
}

string YzHelper::locationStruct2string(JNIEnv* env, jobject loc) {
	MSG_DWORD upload_alarm_status = 0;
	MSG_DWORD upload_status = 0;
	MSG_BCD upload_time[6];
	MSG_DWORD upload_longitude;
	MSG_DWORD upload_latitude;
	MSG_WORD upload_altitude;
	MSG_WORD upload_speed;
	MSG_WORD upload_direction;

	jclass loc_cls = env->GetObjectClass(loc);
	//get alarm status
	jfieldID alarm_status_id = env->GetFieldID(loc_cls, "alarmstatus", "I");
	jint alarm_status = env->GetIntField(loc, alarm_status_id);
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
	//status default set to located
	upload_status = 0x00000002;
	//get longitude
	jfieldID longitude_id = env->GetFieldID(loc_cls, "longitude", "D");
	jdouble longitude = env->GetDoubleField(loc, longitude_id);
	upload_longitude = static_cast<MSG_DWORD>(longitude * 1000000);
	//get latitude
	jfieldID latitude_id = env->GetFieldID(loc_cls, "latitude", "D");
	jdouble latitude = env->GetDoubleField(loc, latitude_id);
	upload_latitude = static_cast<MSG_DWORD>(latitude * 1000000);
	//get altitude
	jfieldID altitude_id = env->GetFieldID(loc_cls, "altitude", "F");
	jfloat altitude = env->GetFloatField(loc, altitude_id);
	upload_altitude = static_cast<MSG_WORD>(altitude);
	//get speed
	jfieldID speed_id = env->GetFieldID(loc_cls, "speed", "F");
	jfloat speed = env->GetFloatField(loc, speed_id);
	upload_speed = static_cast<MSG_WORD>(speed);
	//get direction
	jfieldID direction_id = env->GetFieldID(loc_cls, "direction", "F");
	jfloat direction = env->GetFloatField(loc, direction_id);
	upload_direction = static_cast<MSG_WORD>(direction);
	//get time
	jfieldID time_id = env->GetFieldID(loc_cls, "time", "J");
	jlong time = env->GetLongField(loc, time_id);
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
	message += big_endian(upload_latitude);
	message += big_endian(upload_longitude);
	message += big_endian(upload_altitude);
	message += big_endian(upload_speed);
	message += big_endian(upload_direction);
	message.append((char*) &upload_time[0], 6);
	return message;
}

jint YzHelper::getIntField(JNIEnv *env, jclass cls, jobject obj, const char* field) {
	jfieldID field_id = env->GetFieldID(cls, field, "I");
	return env->GetIntField(obj, field_id);
}

void YzHelper::setIntField(JNIEnv *env, jclass cls, jobject obj, const char* field,
		jint value) {
	jfieldID field_id = env->GetFieldID(cls, field, "I");
	env->SetIntField(obj, field_id, value);
}

jlong YzHelper::getLongField(JNIEnv *env, jclass cls, jobject obj, const char* field) {
	jfieldID field_id = env->GetFieldID(cls, field, "J");
	return env->GetLongField(obj, field_id);
}

void YzHelper::setLongField(JNIEnv *env, jclass cls, jobject obj, const char* field,
		jlong value) {
	jfieldID field_id = env->GetFieldID(cls, field, "J");
	env->SetLongField(obj, field_id, value);
}

jdouble YzHelper::getDoubleField(JNIEnv *env, jclass cls, jobject obj,
		const char* field) {
	jfieldID field_id = env->GetFieldID(cls, field, "D");
	return env->GetDoubleField(obj, field_id);
}

void YzHelper::setDoubleField(JNIEnv *env, jclass cls, jobject obj, const char* field,
		jdouble value) {
	jfieldID field_id = env->GetFieldID(cls, field, "D");
	env->SetDoubleField(obj, field_id, value);
}

jfloat YzHelper::getFloatField(JNIEnv *env, jclass cls, jobject obj, const char* field) {
	jfieldID field_id = env->GetFieldID(cls, field, "F");
	return env->GetFloatField(obj, field_id);
}
void YzHelper::setFloatField(JNIEnv *env, jclass cls, jobject obj, const char* field,
		jfloat value) {
	jfieldID field_id = env->GetFieldID(cls, field, "F");
	env->SetFloatField(obj, field_id, value);
}

jstring YzHelper::getStringField(JNIEnv *env, jclass cls, jobject obj,
		const char* field) {
	jfieldID field_id = env->GetFieldID(cls, field, "Ljava/lang/String;");
	return (jstring)env->GetObjectField(obj, field_id);
}

void YzHelper::setStringField(JNIEnv *env, jclass cls, jobject obj, const char* field,
		jstring value) {
	jfieldID field_id = env->GetFieldID(cls, field, "Ljava/lang/String;");
	env->SetObjectField(obj, field_id, value);
}

string YzHelper::vehicleDataStruct2string(JNIEnv* env, jobject vehicle_data) {
	string message = "";
	jclass cls = env->GetObjectClass(vehicle_data);
	jint speed = getIntField(env, cls, vehicle_data, "speed");
	jint oilLevel = getIntField(env, cls, vehicle_data, "oilLevel");
	jint ACCSwitch = getIntField(env, cls, vehicle_data, "ACCSwitch");
	jint located = getIntField(env, cls, vehicle_data, "located");
	jint operating = getIntField(env, cls, vehicle_data, "operating");
	jint oilCircuit = getIntField(env, cls, vehicle_data, "oilCircuit");
	jint electricCircuit = getIntField(env, cls, vehicle_data,
			"electricCircuit");
	jint doorLocked = getIntField(env, cls, vehicle_data, "doorLocked");
	jlong mileage = getLongField(env, cls, vehicle_data, "mileage");
	MSG_DWORD status = 0;
	status = (MSG_DWORD) ((ACCSwitch ? 0x0001 : 0x0000)
			| (located ? 0x0002 : 0x0000) | (operating ? 0x0010 : 0x0000)
			| (oilCircuit ? 0x0400 : 0x0000)
			| (electricCircuit ? 0x0800 : 0x0000)
			| (doorLocked ? 0x1000 : 0x0000));

	MSG_DWORD upload_longitude;
	MSG_DWORD upload_latitude;
	//get latitude
	jdouble latitude = getDoubleField(env, cls, vehicle_data, "latitude");
	upload_latitude = static_cast<MSG_DWORD>(latitude * 1000000);
	//get longitude
	jdouble longitude = getDoubleField(env, cls, vehicle_data, "longitude");
	upload_longitude = static_cast<MSG_DWORD>(longitude * 1000000);

	//get time
	MSG_BCD upload_time[6];
	jlong time = getLongField(env, cls, vehicle_data, "time");
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
	message += big_endian((MSG_DWORD) 0); //报警
	message += big_endian(status);
	message += big_endian(upload_latitude); //纬度
	message += big_endian(upload_longitude); //经度
	message += big_endian((MSG_WORD) 0); //高程
	message += big_endian((MSG_WORD) speed);
	message += big_endian((MSG_WORD) 0); //方向
	message.append((char*) &upload_time[0], 6);
//	message += big_endian((MSG_DWORD) 0); //时间前4个字节
//	message += big_endian((MSG_WORD) 0); //时间后2个字节
	message += (MSG_BYTE) 0x01; //里程
	message += (MSG_BYTE) 4;
	message += big_endian((MSG_DWORD) mileage);
	message += (MSG_BYTE) 0x02;
	message += (MSG_BYTE) 2;
	message += big_endian((MSG_WORD) oilLevel);
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x20;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x02;
//	message += (char) 0x0B;
//	message += (char) 0x76;
//	message += (char) 0xF1;
//	message += (char) 0x06;
//	message += (char) 0xFB;
//	message += (char) 0x11;
//	message += (char) 0x35;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x13;
//	message += (char) 0x08;
//	message += (char) 0x02;
//	message += (char) 0x23;
//	message += (char) 0x40;
//	message += (char) 0x16;
//	message += (char) 0x01;
//	message += (char) 0x04;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0xBC;
//	message += (char) 0x2D;
//	message += (char) 0x03;
//	message += (char) 0x02;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0xEF;
//	message += (char) 0x04;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0xEA;
//	message += (char) 0x04;
//	message += (char) 0x01;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0xFE;
//	message += (char) 0x02;
//	message += (char) 0xF0;
//	message += (char) 0x00;
//	message += (char) 0xFF;
//	message += (char) 0x06;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x00;
//	message += (char) 0x01;
//	message += (char) 0x0F;
//	message += (char) 0xFF;

	return message;
}

string YzHelper::jbyteArray2string(JNIEnv *env, jbyteArray array) {
	unsigned int len = env->GetArrayLength(array);
	jbyte *buf = new (nothrow) jbyte[len + 1];
	if (buf == NULL) {
		return string(1, (char) 0);
	}
	env->GetByteArrayRegion(array, 0, len, buf);
	buf[len] = 0;
	string str((char*) buf, len + 1);
	delete[] buf;
	return str;
}

jobject YzHelper::msg2poi(JNIEnv* env, const msg_body_t & msg) {
	jobject poi = env->AllocObject(Middleware::g_poi_cls);
	jclass cls = env->GetObjectClass(poi);
	MSG_DWORD latitude, longtitude;
	jfloat jlatitude, jlongtitude;
	big_endian2dword(msg.content, &latitude);
	big_endian2dword(msg.content + sizeof(MSG_DWORD), &longtitude);
	jlatitude = static_cast<jfloat>(latitude) / 1000000;
	jlongtitude = static_cast<jfloat>(longtitude) / 1000000;
	setFloatField(env, cls, poi, "latitude", jlatitude);
	setFloatField(env, cls, poi, "longtitude", jlongtitude);
	logcatf("msg length = %u\n", msg.length);
	jstring jpoi_name;
	if (msg.length > 2 * sizeof(MSG_DWORD)) {
		jclass strClass = env->FindClass("java/lang/String");
		jmethodID ctorID = env->GetMethodID(strClass, "<init>",
				"([BLjava/lang/String;)V");
		jbyteArray bytes = env->NewByteArray(
				msg.length - 2 * sizeof(MSG_DWORD));
		env->SetByteArrayRegion(bytes, 0, msg.length - 2 * sizeof(MSG_DWORD),
				(jbyte*) msg.content +  2 * sizeof(MSG_DWORD));
		jstring encoding = env->NewStringUTF("gb2312");
		jpoi_name = (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
	} else {
		jpoi_name = string2jstring(env, "");
	}
	setStringField(env, cls, poi, "poiname", jpoi_name);
	logcatf("latitude:%u\tlongtitude%u\t", latitude, longtitude);
	return poi;
}
