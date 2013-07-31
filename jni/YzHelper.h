/*
 * YzErr.h
 *
 *  Created on: Jul 17, 2013
 *      Author: lookflying
 */
#include <jni.h>
#include <string>

#include "message/message.h"
#include "Connection.h"
#include "ConnectionManager.h"

#ifndef YZERR_H_
#define YZERR_H_

#define YZ_OK			(0)		//成功
#define YZ_CON_CLOSED	(-2)	//连接已关闭
#define YZ_CON_TIMEOUT	(-1)	//连接超时
#define YZ_NOT_LOGIN	(1001)	//未登录
#define YZ_LOGIN_FAIL	(1002)	//登录失败
#define YZ_LOGOUT_FAIL	(1003)	//注销失败
#define YZ_W_GPS_FAIL	(1004)	//写入GPS数据失败
#define YZ_DUP_LOGIN	(1006)	//重复登录
#define YZ_GPS_INVALID	(1007)	//GPS数据无效
#define YZ_USER_EXIST	(1008)	//登录的用户已存在
#define YZ_DEVICE_NOT_REGIST	(1009)//终端未被正确注册
#define YZ_DEVICE_NOT_BINDING	(1010)//终端未绑定有效车辆

#define YZ_UNEXPECTED	9997
#define YZ_SOCK_ERROR  9998
#define YZ_OUT_OF_MEM  9999

#define YZMSGID_GENERAL_TERMINAL_RESPONSE		0X0001
#define YZMSGID_GENERAL_PLATFORM_RESPONSE		0X8001
#define YZMSGID_TERMINAL_HEARTBEAT			0X0002
#define YZMSGID_TERMINAL_REGISTER				0X0100
#define YZMSGID_TERMINAL_REGISTER_RESPONSE	0X8100
#define YZMSGID_TERMINAL_DEREGISTER				0X0003
#define YZMSGID_TERMINAL_AUTHORIZE			0x0102
#define YZMSGID_SET_TERMINAL_PARAMETER		0X8103
#define YZMSGID_QUERY_TERMIAL_PARATETER		0X8104
#define YZMSGID_QUERY_TERMIAL_PARATETER_RESPONSE	0X0104
#define YZMSGID_TERMINAL_CONTROL				0X8105
#define YZMSGID_POSITION_REPORT				0X0200//位置信息汇报
#define YZMSGID_POSISION_QUERY				0X8201
#define YZMSGID_POSISION_QUERY_RESPONSE		0X0201
#define YZMSGID_TMP_POSITION_CONTROL			0X8202
#define YZMSGID_TEXT_DISPATCH					0X8300
#define YZMSGID_SET_EVENT						0X8301
#define YZMSGID_EVENT_REPORT					0X0301
#define YZMSGID_DISPATCH_ASK					0X8302
#define YZMSGID_ASK_RESPONSE					0X0302



/**
 * printf to logcat
 */
void logcatf(const char* fmt, ...);
/**
 * print hex
 */
void logcat_hex(const char* buf, unsigned int len);
/**
 * jstring to string
 */
std::string jstring2string(JNIEnv* env, jstring jstr);
/**
 * string to jstring
 */
jstring string2jstring(JNIEnv* env, const std::string str);


/**
 * 将string中的内容封装为msg_body_t, 不进行内存拷贝
 */
msg_body_t string2msg_body(std::string content);

/**
 * 将字符串复制到指定长度得buf中，字符串长度不足时，自动补0
 */
void string2bytes(const std::string str, MSG_BYTE* target, unsigned int len);

/**
 * 序列化LocationStruct至string
 */
std::string locationStruct2string(JNIEnv* env, jobject loc);

/**
 *序列化VehicleDataStruct至string
 */
std::string vehicleDataStruct2string(JNIEnv* env, jobject vehicle_data);

/**
 *获取对象int型字段
 */
jint getIntField(JNIEnv *env, jclass cls, jobject obj, const char* field);
/**
 *获取对象long型字段

 */
jlong getLongField(JNIEnv *env, jclass cls, jobject obj, const char* field);
/**
 * 将字节数组转换为string
 */
std::string jbyteArray2string(JNIEnv *env, jbyteArray array);
#endif /* YZERR_H_ */
