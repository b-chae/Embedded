#include <jni.h>
#include "android/log.h"
#include <unistd.h>
#include <fcntl.h>

#define LOG_TAG "MyTag"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

extern int first(int x,int y);

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_driveropen(JNIEnv *env, jobject this){
	int fd = open("/dev/fpga_led_driver", O_RDWR);
	return fd;
}

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_switchdriveropen(JNIEnv *env, jobject this){
	int fd = open("/dev/fpga_push_switch", O_RDWR);
	LOGV("fpga_push_switch %d", fd);
	return fd;
}

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_driverclose(JNIEnv *env, jobject this, jint fd){
	return close(fd);
}

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_switchdriverclose(JNIEnv *env, jobject this, jint fd){
	return close(fd);
}

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_driverwrite(JNIEnv *env, jobject this, jint fd, jint mode, jstring value){
	char buf[200];
	const char *str=(*env)->GetStringUTFChars(env,value,0);
	unsigned char data;
	int len;
	int i,result;
	for(i=0;;i++){
		if(str[i]=='\0') {len=i; break;}
	}
	if(mode==20){
		data = 0;
		result=write(fd,&data,1);
	}
	else if(mode==21){
		buf[0]=2; buf[1]=1;
		for(i=2;i<len+2;i++){
			buf[i]=str[i-2];
		}
		result=write(fd,buf,len+2);
	}
	else{
		buf[0]=mode;
		for(i=1;i<len+1;i++){
			buf[i]=str[i-1];
		}
		result=write(fd,buf,len+1);
	}
	return result;
}

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_switchread(JNIEnv *env, jobject this, jint fd){
	unsigned char sw_buff[9];
	int result = 0;
	int i;

	read(fd, &sw_buff, sizeof(sw_buff));
	for(i=0; i<9; i++){
		if(sw_buff[i] == 1){
			result = i+1;
			break;
		}
	}

	return result;
}

jint JNICALL Java_org_example_ndk_NDKExam_add(JNIEnv *env, jobject this, jint x, jint y)
{
	LOGV("log test %d", 1234);
	return first(x, y);
}

void JNICALL Java_org_example_ndk_NDKExam_testString(JNIEnv *env, jobject this, jstring string)
{
	const char *str=(*env)->GetStringUTFChars( env, string, 0);
	jint len = (*env)->GetStringUTFLength( env, string );
	LOGV("native testString len %d", len);
	LOGV("native testString %s", str);
	
	(*env)->ReleaseStringUTFChars( env, string, str );	
}
