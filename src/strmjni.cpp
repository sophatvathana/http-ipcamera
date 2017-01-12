/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:52:49
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:20:47
*/
#define _GLIBCXX_USE_CXX11_ABI 0
#include <jni.h>
#include <strmrecvclientapi.h>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;
//#pragma GCC java_exceptions
/*
* Class:     com_innodep_RtspStream
* Method:    createContextInternal
* Signature: (Ljava/lang/String;)J
*/
extern "C" JNIEXPORT jlong JNICALL Java_kh_com_kshrd_core_RtspStream_createContextInternal
(JNIEnv *env, jobject thisObj, jstring path) {
	jlong        result  = 0;
	const char* lpszPath = NULL;
	lpszPath = env->GetStringUTFChars(path, NULL);
	void*  context = ::StrmCreateContext(lpszPath);
	env->ReleaseStringUTFChars(path, lpszPath);
	if (context)
		result = (jlong)context;
  
	return result;
}

/*
* Class:     com_innodep_RtspStream
* Method:    openRtspInternal
* Signature: (JLjava/lang/String;)Z
*/
extern "C" JNIEXPORT jboolean JNICALL Java_kh_com_kshrd_core_RtspStream_openRtspInternal
(JNIEnv *env, jobject thisObj, jlong handler, jstring url) {
	jboolean      result    = JNI_FALSE;
	const char*   szUrl     = NULL;
	std::string   path;

	szUrl = env->GetStringUTFChars(url, NULL);
	path  = szUrl;

	void* context = (void*)handler;
	env->ReleaseStringUTFChars(url, szUrl);
	
	if (::StrmOpenRtsp(context, path.c_str()) != 0)
		result = JNI_TRUE;
	//strmrecvclient_start_log("", "");
	// if (::strmrecvclient_start(handler, path.c_str(),1) !=-1)
	// 	result = JNI_TRUE;
	return result;
}

/*
* Class:     com_innodep_RtspStream
* Method:    readFrameInternal
* Signature: (J)[B
*/
extern "C" JNIEXPORT jbyteArray JNICALL Java_kh_com_kshrd_core_RtspStream_readFrameInternal
(JNIEnv *env, jobject thisObj, jlong handler, jstring url) {
	jbyteArray result = NULL;
	int bufferLen = 0;
	const char*   szUrl     = NULL;
	std::string   path;

	szUrl = env->GetStringUTFChars(url, NULL);
	path  = szUrl;

	void* context = (void*)handler;
	env->ReleaseStringUTFChars(url, szUrl);

	unsigned char* buffer = StrmReadFrame(0, path.c_str());

	if (buffer && bufferLen > 0) {
		result = env->NewByteArray(bufferLen);
		if (result) {
			jbyte* resultPtr = env->GetByteArrayElements(result, NULL);
			std::memcpy(resultPtr, buffer, bufferLen);
			env->ReleaseByteArrayElements(result, resultPtr, 0);
		}
	}
	return result;
}

/*
* Class:     com_innodep_RtspStream
* Method:    releaseContextInternal
* Signature: (J)V
*/
extern "C" JNIEXPORT void JNICALL Java_kh_com_kshrd_core_RtspStream_releaseContextInternal
(JNIEnv *env, jobject thisObj, jlong handle) {
	void* context = (void*)handle;
	if (context)
		StrmReleaseContext(context);
}