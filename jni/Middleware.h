/*
 * Middleware.h
 *
 *  Created on: Dec 3, 2013
 *      Author: lookflying
 */

#ifndef MIDDLEWARE_H_
#define MIDDLEWARE_H_
#include <string>
#include "YzHelper.h"
#include "Connection.h"
#include "ConnectionManager.h"
/**
 * 登录状态结构体
 */
typedef struct log_state {
	std::string simcardnum;
	long userId;
	std::string authCode;
	int state;
} log_state_t;
class Middleware {
	public:
		Middleware();
		virtual ~Middleware();

		static ConnectionManager g_conn_manager;
		static JavaVM * g_jvm;
		static jobject g_listen_obj;
		static jclass g_poilist_cls;
		static jclass g_tmc_struct_cls;
		static jclass g_weather_struct_cls;
		static jclass g_poi_cls;
		static bool g_working;
		/**
		 * 启动化中间件
		 */
		static void startMiddleware(const std::string server_ip, const int server_port);
		/**
		 * 停止中间件
		 */
		static void stopMiddleware();

		/**
		 * 设置全局jvm
		 */
		static void setJVM(JNIEnv *env);

		/**
		 * 清楚全局jvm
		 */
		static void unsetJVM();
		/**
		 * 设置全局监听java对象
		 */
		static void setListen(JNIEnv *env, jobject listen);
		/**
		 * 设置全局监听java对象
		 */
		static void unsetListen(JNIEnv *env);

		/**
		 * 调用java void函数,支持直接传参数
		 */
		static void callVoidMethod(JNIEnv *env, jclass cls, const char* name, const char* sig,
				...);

		/**
		 * 登录状态回调函数
		 */
		static void logStateCallBack(std::string simcardnum, long userId, std::string authCode,
				int state);

		/**
		 * 清理jclass
		 */
		static void unsetCls(JNIEnv *env);

		/**
		 * 登录函数回调routine
		 */
		static void* logStateCallBackWorker(void* p);

		/**
		 * 从msg_body获取poi struct
		 */
		jobject msg2poi(JNIEnv* env,const msg_body_t &msg);
};

#endif /* MIDDLEWARE_H_ */
