#include "YzHelper.h"
#include "Connection.h"
#include <unistd.h>
#include <cstring>
#include <android/log.h>
#define EV_STANDALONE 1
#include "ev.h"
using namespace std;
//helper
string jstring2string(JNIEnv* env, jstring jstr){
	char* temp = NULL;
	string str = "";
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* ba = env->GetByteArrayElements(barr, NULL);
	if (alen > 0){
		temp = new (nothrow) char[alen];
		memcpy(temp, ba, alen);
	}
	str.append(temp, alen);
	delete[] temp;
	env->ReleaseByteArrayElements(barr, ba, 0);
	return str;
}

//char* to jstring
jstring string2jstring(JNIEnv* env, const string str){
	jclass strClass = env->FindClass("Ljava/lang/String;");
	jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(str.size());
	env->SetByteArrayRegion(bytes, 0, str.size(), (jbyte*)&str.c_str()[0]);
	jstring encoding = env->NewStringUTF("utf-8");
	return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
}

void testHandler(const Connection *conn, const char *buf, int len)
{
}

void closedHandler(const Connection &conn)
{
}

void *worker(void *param)
{
	struct ev_loop *loop = static_cast<struct ev_loop*>(param);
	ev_loop(loop, 0);
	return NULL;
}

void idle_cb(struct ev_loop *loop, struct ev_idle *w, int revents)
{
	sleep(1);
}


jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	struct ev_loop *loop = ev_default_loop(0);
	struct ev_idle *idle_watcher = (ev_idle*) (malloc(sizeof(struct ev_idle)));
	ev_idle_init(idle_watcher, idle_cb);
	ev_idle_start(loop, idle_watcher);

	MSG_WORD platformResponseMsgIds[] = {YZMSGID_GENERAL_PLATFORM_RESPONSE, YZMSGID_TERMINAL_REGISTER_RESPONSE};
	Connection::responseMsgIdSet.insert(platformResponseMsgIds, platformResponseMsgIds + sizeof(platformResponseMsgIds) / sizeof(platformResponseMsgIds[0]));

	Connection conn("test connection", loop);
	conn.initServerAddr("121.101.223.68", 6973);
	conn.setAuthorizationCode("12345");
	conn.setClosedHandler(closedHandler);

	pthread_t tid;
	pthread_create(&tid, NULL, worker, loop);
	sleep(1);
	__android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "\n hello world, form onload\n");
	return JNI_VERSION_1_6;

}

void JNI_OnUnload(JavaVM *vm, void *reserved) {

}
