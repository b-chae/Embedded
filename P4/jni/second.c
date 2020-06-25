#include <jni.h>
#include "android/log.h"
#include <unistd.h>
#include <fcntl.h>

#define LOG_TAG "MyTag"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

extern int first(int x,int y);

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_switchdriveropen(JNIEnv *env, jobject this){
	int fd = open("/dev/fpga_push_switch", O_RDWR);
	return fd;
}

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_switchdriverclose(JNIEnv *env, jobject this, jint fd){
	return close(fd);
}

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_driverwrite(JNIEnv *env, jobject this, jint value){

	int fd = open("/dev/fpga_led_driver", O_RDWR);

	unsigned char data;
	int result;

	switch(value){
		case 0 : data = 0; break;
		case 1 : data = 1; break;
		case 2 : data = 3; break;
		case 3 : data = 7; break;
		case 4 : data = 15; break;
		case 5 : data = 31; break;
		case 6 : data = 63; break;
		case 7 : data = 127; break;
		case 8 : data = 255; break;
		default : data = 255; break;
	}

	result = write(fd, &data, 1);

	close(fd);

	return result;
}

JNIEXPORT jint JNICALL Java_org_example_ndk_NDKExam_fndwrite(JNIEnv *env, jobject this, jint value){

	int fd = open("/dev/fpga_fnd", O_RDWR);

	unsigned char data[4];
	int result;

	data[3] = value%10;
	data[2] = value%100/10;
	data[1] = value/100%10;
	data[0] = value/1000;

	result = write(fd, &data, 4);

	close(fd);

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
