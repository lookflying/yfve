#include "YzHelper.h"
#include <cstring>
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

jint JNI_OnLoad(JavaVM* vm, void* reserved) {

		return JNI_VERSION_1_6;

}

void JNI_OnUnload(JavaVM *vm, void *reserved) {

}
