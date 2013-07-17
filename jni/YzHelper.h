/*
 * YzErr.h
 *
 *  Created on: Jul 17, 2013
 *      Author: lookflying
 */
#include <jni.h>
#include <string>
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

std::string jstring2string(JNIEnv* env, jstring jstr);
jstring string2jstring(JNIEnv* env, const std::string str);
#endif /* YZERR_H_ */
