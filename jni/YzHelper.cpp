#include "YzHelper.h"
#include <unistd.h>
#include <cstring>
#include <android/log.h>
#define EV_STANDALONE 1
#include "ev.h"
using namespace std;

/**
 * logcat buffer
 */
char logcat_buf[1024];

/**
 * printf to logcat
 */
void logcatf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsnprintf(logcat_buf, sizeof(logcat_buf), fmt, args);
	va_end(args);
	__android_log_write(ANDROID_LOG_DEBUG, "logcatf", logcat_buf);
}

/**
 * jstring to string
 */
string jstring2string(JNIEnv* env, jstring jstr) {
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
jstring string2jstring(JNIEnv* env, const string str) {
	jclass strClass = env->FindClass("java/lang/String");
	jmethodID ctorID = env->GetMethodID(strClass, "<init>",
			"([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(str.size());
	env->SetByteArrayRegion(bytes, 0, str.size(), (jbyte*) &str.c_str()[0]);
	jstring encoding = env->NewStringUTF("utf-8");
	return (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
}


msg_body_t string2msg_body(std::string content){
	msg_body_t body;
	body.content = (MSG_BYTE*)&content.c_str()[0];
	body.length = content.size();
	return body;
}
void string2bytes(const std::string str, MSG_BYTE* target, unsigned int len){
	if (str.size() >= len){
		memcpy(target, str.c_str(), len);
	}else{
		memcpy(target, str.c_str(), str.size());
		memset(target + str.size(), 0, len - str.size());
	}
}
ConnectionManager g_conn_manager;

bool messageHandler(const Connection &conn, MSG_WORD msgid, MSG_WORD msgSerial,
		const msg_body_t &msg, MSG_WORD *responseMsgid, string *response) {
	return false;
}

void connClosedHandler(const Connection &conn){

}

void initYzService(const std::string server_ip, const int server_port) {
	(void) g_conn_manager.start();
	Connection &g_connection = *g_conn_manager.getConnection(0);
	g_connection.initServerAddr(server_ip, server_port);
	g_connection.setMessageHandler(messageHandler);
	g_connection.setClosedHandler(connClosedHandler);

}

