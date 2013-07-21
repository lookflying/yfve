/*
 * middleware.h
 *
 *  Created on: Jul 20, 2013
 *      Author: lookflying
 */

#include <string>
#include "YzHelper.h"
#include "Connection.h"
#include "ConnectionManager.h"
#ifndef MIDDLEWARE_H_
#define MIDDLEWARE_H_

extern ConnectionManager g_conn_manager;
extern JavaVM * g_jvm;
extern jclass g_poilist_cls;
extern jclass g_tmc_struct_cls;
extern jclass g_weather_struct_cls;
/**
 * 启动化中间件
 */
void startMiddleware(const std::string server_ip, const int server_port);
/**
 * 停止中间件
 */
void stopMiddleware();

/**
 * 设置全局jvm
 */
void setJVM(JNIEnv *env);

/**
 * 清楚全局jvm
 */
void unsetJVM();
/**
 * 设置全局监听java对象
 */
void setListen(JNIEnv *env, jobject listen);
/**
 * 设置全局监听java对象
 */
void unsetListen(JNIEnv *env);

/**
 * 调用java void函数,支持直接传参数
 */
void callVoidMethod(JNIEnv *env, jclass cls, const char* name, const char* sig,
		...);

/**
 * 清理jclass
 */
void unsetCls(JNIEnv *env);

#endif /* MIDDLEWARE_H_ */
