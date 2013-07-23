/*
 * ConnectionHelper.cpp
 *
 *  Created on: Jul 20, 2013
 *      Author: lookflying
 */

#include "middleware.h"
#include <cstdarg>
using namespace std;

ConnectionManager g_conn_manager;

bool g_working = false;
JavaVM * g_jvm = NULL;

jobject g_listen_obj = NULL;

jclass g_poilist_cls = NULL;
jclass g_tmc_struct_cls = NULL;
jclass g_weather_struct_cls = NULL;

/**
 * 服务器主发消息回调函数
 * 需要根据协议在这里为每一种服务器主发得消息id分别设置所需要回调得java函数
 * 并需要根据协议，生成应答消息，设置reponseMsgid及response消息体内容。
 * 由于目前尚未确定回调函数对应得服务器主发消息id，所以这里将所有回调函数都调用一遍，
 * 并发送终端终端通用应答
 * 以供测试
 * 2013.7.21
 */

void callVoidMethod(JNIEnv *env, jclass cls, const char* name, const char* sig,
		...) {
	if (g_listen_obj != NULL) {
		jmethodID mid = env->GetMethodID(cls, name, sig);
		va_list args;
		va_start(args, sig);
		if (mid != NULL) {
			env->CallVoidMethodV(g_listen_obj, mid, args);
		}
		va_end(args);
	}
}

jobject callDefaultConstructor(JNIEnv *env, jclass cls){
	jmethodID mid = env->GetMethodID(cls,"<init>","()V");
	return env->NewObject(cls, mid);
}

jobject getNewObject(JNIEnv *env, const char* name) {
	jclass cls = env->FindClass(name);
	return env->AllocObject(cls);
}

bool messageHandler(const Connection &conn, MSG_WORD msgid, MSG_WORD msgSerial,
		const msg_body_t &msg, MSG_WORD *responseMsgid, string *response) {
	logcatf("got message id = %u size = %u", msgid, msg.length);
	JNIEnv *env = NULL;
	jclass cls = NULL;
	jmethodID mid;
	if (g_jvm != NULL && g_jvm->AttachCurrentThread(&env, NULL) == JNI_OK
			&& g_listen_obj != NULL) {
		cls = env->GetObjectClass(g_listen_obj);
		switch (msgid) {
		default:
			jobject poilist = env->AllocObject(g_poilist_cls);
			jint poinum = 0;
			jint result = -1;
			callVoidMethod(env, cls, "yz_3_remotedescallback",
					"(ILjava/util/List;I)V", poinum, poilist, result);
			jobject tmc_struct = env->AllocObject(g_tmc_struct_cls);
			callVoidMethod(env, cls, "yz_3_TMCcallback",
					"(Lvehicle_CVS/TMCStruct_DSP;)V", tmc_struct);


			jobject weather_struct = env->AllocObject(g_weather_struct_cls);
			callVoidMethod(env, cls, "yz_3_weathercallback",
					"(Lvehicle_CVS/WeatherStruct_DSP;)V", weather_struct);
			break;
		}
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			logcatf("middleware", "DetachCurrentThread fail");
		}

	}
	*responseMsgid = (MSG_WORD) YZMSGID_GENERAL_TERMINAL_RESPONSE;
	*response = "";
	*response += big_endian((MSG_WORD) msgSerial);
	*response += big_endian((MSG_WORD) msgid);
	*response += (char) 0;
	return true; //若返回false则不发送应答
}

void* logStateCallBackWorker(void* p) {
	log_state_t* state_p = (log_state_t*) p;
	logStateCallBack(state_p->simcardnum, state_p->userId, state_p->authCode,
			state_p->state);
	return (void*) 0;
}

void logStateCallBack(string simcardnum, long userId, string authCode,
		int state) {
	if (g_jvm != NULL && g_listen_obj != NULL) {
		JNIEnv *env = NULL;
		jclass cls = NULL;
		jmethodID mid;
		if (g_jvm->AttachCurrentThread(&env, NULL) == JNI_OK) {
			cls = env->GetObjectClass(g_listen_obj);
			if (cls != NULL) {
				mid = env->GetMethodID(cls, "yz_3_loginstatecallback",
						"(Ljava/lang/String;JLjava/lang/String;I)V");
				if (mid != NULL) {
					jstring jsimcardnum = string2jstring(env, simcardnum);
					jlong juserId = (jlong) userId;
					jstring jauthCode = string2jstring(env, authCode);
					jint jstate = (jint) state;
					env->CallVoidMethod(g_listen_obj, mid, jsimcardnum, juserId,
							jauthCode, jstate);
				}
			}
			if (g_jvm->DetachCurrentThread() != JNI_OK) {
				logcatf("middleware", "DetachCurrentThread fail");
			}
		}
	}
}

void connClosedHandler(const Connection &conn) {
	logStateCallBack("", 0, "", 1);
}

void startMiddleware(const std::string server_ip, const int server_port) {
	MSG_WORD platformResponseMsgIds[] = {
			YZMSGID_GENERAL_PLATFORM_RESPONSE,
			YZMSGID_TERMINAL_REGISTER_RESPONSE,

			};
	Connection::responseMsgIdSet.insert(platformResponseMsgIds,
			platformResponseMsgIds
					+ sizeof(platformResponseMsgIds)
							/ sizeof(platformResponseMsgIds[0]));

	(void) g_conn_manager.start();

	Connection &g_connection = *g_conn_manager.getConnection(0);
	g_connection.initServerAddr(server_ip, server_port);
	g_connection.setMessageHandler(messageHandler);
	g_connection.setClosedHandler(connClosedHandler);
	g_working = true;
}

void setJVM(JNIEnv *env) {
	env->GetJavaVM(&g_jvm);
}

void stopMiddleware() {
	(void) g_conn_manager.stop();
	g_working = false;
}

void unsetJVM() {
//	g_jvm->DestroyJavaVM();
	g_jvm = NULL;
}

void setListen(JNIEnv *env, jobject listen) {
	g_listen_obj = env->NewGlobalRef(listen);
}

void unsetListen(JNIEnv *env) {
	env->DeleteGlobalRef(g_listen_obj);
}

void unsetCls(JNIEnv *env) {
	env->DeleteGlobalRef(g_poilist_cls);
	env->DeleteGlobalRef(g_tmc_struct_cls);
	env->DeleteGlobalRef(g_weather_struct_cls);
}
