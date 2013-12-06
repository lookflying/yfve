/*
 * Middleware.cpp
 *
 *  Created on: Dec 3, 2013
 *      Author: lookflying
 */

#include "Middleware.h"
using namespace std;
ConnectionManager Middleware::g_conn_manager;
bool Middleware::g_working = false;
JavaVM * Middleware::g_jvm = NULL;
jobject Middleware::g_listen_obj = NULL;
jclass Middleware::g_poilist_cls = NULL;
jclass Middleware::g_tmc_struct_cls = NULL;
jclass Middleware::g_weather_struct_cls = NULL;
jclass Middleware::g_poi_cls = NULL;
Middleware::Middleware() {
}

Middleware::~Middleware() {
	// TODO Auto-generated destructor stub
}

void Middleware::callVoidMethod(JNIEnv *env, jclass cls, const char* name,
		const char* sig, ...) {
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

jobject callDefaultConstructor(JNIEnv *env, jclass cls) {
	jmethodID mid = env->GetMethodID(cls, "<init>", "()V");
	return env->NewObject(cls, mid);
}

jobject getNewObject(JNIEnv *env, const char* name) {
	jclass cls = env->FindClass(name);
	return env->AllocObject(cls);
}

bool messageHandler(const Connection &conn, MSG_WORD msgid, MSG_WORD msgSerial,
		const msg_body_t &msg, MSG_WORD *responseMsgid, string *response) {
	YzHelper::logcatf("got message id = %u size = %u", msgid, msg.length);
	JNIEnv *env = NULL;
	jclass cls = NULL;
	jmethodID mid;
	bool attached = false;
	if (Middleware::g_jvm != NULL && Middleware::g_listen_obj != NULL) {
		switch (Middleware::g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6)) {
		case JNI_OK:
			break;
		case JNI_EDETACHED:
			if (Middleware::g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
				return false;
			}
			attached = true;
			break;
		case JNI_EVERSION:
		default:
			return false;
		}

		cls = env->GetObjectClass(Middleware::g_listen_obj);
		YzHelper::logcat_hex((char*) msg.content, msg.length);
		switch (msgid) {
		case (MSG_WORD) YZMSGID_POI: {
			jmethodID list_construct_id = env->GetMethodID(Middleware::g_poilist_cls,
					"<init>", "()V");
			jobject poilist = env->NewObject(Middleware::g_poilist_cls, list_construct_id);
			if (msg.length > 8) {
				jint poinum = 1;
				jint result = 0;
				jobject poi = YzHelper::msg2poi(env, msg);
				jclass array_list_cls = env->GetObjectClass(poilist);
				jmethodID array_list_add_id = env->GetMethodID(array_list_cls,
						"add", "(Ljava/lang/Object;)Z");
				env->CallBooleanMethod(poilist, array_list_add_id, poi);
				Middleware::callVoidMethod(env, cls, "yz_3_remotedescallback",
						"(ILjava/util/List;I)V", poinum, poilist, result);
			}

		}
			break;
		default:
			jobject tmc_struct = env->AllocObject(Middleware::g_tmc_struct_cls);
			Middleware::callVoidMethod(env, cls, "yz_3_TMCcallback",
					"(Lvehicle_CVS/TMCStruct_DSP;)V", tmc_struct);

			jobject weather_struct = env->AllocObject(Middleware::g_weather_struct_cls);
			Middleware::callVoidMethod(env, cls, "yz_3_weathercallback",
					"(Lvehicle_CVS/WeatherStruct_DSP;)V", weather_struct);
			break;
		}
		if (attached && Middleware::g_jvm->DetachCurrentThread() != JNI_OK) {
			YzHelper::logcatf("middleware", "DetachCurrentThread fail");
		}

	}
	*responseMsgid = (MSG_WORD) YZMSGID_GENERAL_TERMINAL_RESPONSE;
	*response = "";
	*response += big_endian((MSG_WORD) msgSerial);
	*response += big_endian((MSG_WORD) msgid);
	*response += (char) 0;
	return true; //若返回false则不发送应答
}

void* Middleware::logStateCallBackWorker(void* p) {
	log_state_t* state_p = (log_state_t*) p;
	logStateCallBack(state_p->simcardnum, state_p->userId, state_p->authCode,
			state_p->state);
	return (void*) 0;
}

void Middleware::logStateCallBack(string simcardnum, long userId,
		string authCode, int state) {
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
					jstring jsimcardnum = YzHelper::string2jstring(env, simcardnum);
					jlong juserId = (jlong) userId;
					jstring jauthCode = YzHelper::string2jstring(env, authCode);
					jint jstate = (jint) state;
					env->CallVoidMethod(g_listen_obj, mid, jsimcardnum, juserId,
							jauthCode, jstate);
				}
			}
			if (g_jvm->DetachCurrentThread() != JNI_OK) {
				YzHelper::logcatf("middleware", "DetachCurrentThread fail");
			}
		}
	}
}

void connClosedHandler(Connection *conn) {
	Middleware::logStateCallBack("", 0, "", 1);
}

void Middleware::startMiddleware(const std::string server_ip,
		const int server_port) {
	MSG_WORD platformResponseMsgIds[] = { YZMSGID_GENERAL_PLATFORM_RESPONSE,
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

void Middleware::setJVM(JNIEnv *env) {
	env->GetJavaVM(&g_jvm);
}

void Middleware::stopMiddleware() {
	(void) g_conn_manager.stop();
	g_working = false;
}

void Middleware::unsetJVM() {
	//	g_jvm->DestroyJavaVM();
	g_jvm = NULL;
}

void Middleware::setListen(JNIEnv *env, jobject listen) {
	g_listen_obj = env->NewGlobalRef(listen);
}

void Middleware::unsetListen(JNIEnv *env) {
	env->DeleteGlobalRef(g_listen_obj);
}

void Middleware::unsetCls(JNIEnv *env) {
	env->DeleteGlobalRef(g_poilist_cls);
	env->DeleteGlobalRef(g_tmc_struct_cls);
	env->DeleteGlobalRef(g_weather_struct_cls);
}
