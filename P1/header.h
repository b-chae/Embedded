#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/mman.h>

#define KEY_RELEASE 0
#define KEY_PRESS 1

#define FND_DEVICE "/dev/fpga_fnd"
#define SWITCH_DEVICE "/dev/fpga_push_switch"
#define DOT_DEVICE "/dev/fpga_dot"
#define TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
#define EVENT_DEVICE "/dev/input/event0"

#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16 

#define CLOCK_MODE 0
#define COUNTER_MODE 1
#define TEXT_MODE 2
#define DRAW_MODE 3

#define SWITCH 1
#define EVENT 2
#define FND 10
#define FND_WITH_BASE 11
#define DOT 12
#define LED 13

/*struct definition*/
/* switbuf
 * long type : 항상 SWITCH
 * int n : SWITCH가 동시에 눌려지는 개수 (1일경우 하나의 스위치가, 2일 경우 두개의 스위치가 눌려짐을 뜻한다.)
 * unsigned char value[9] : 9개의 switch에 대한 값, 0은 안눌려짐, 1은 눌려짐을 뜻한다.
 */
struct switbuf{
	long type;
	int n;
	unsigned char value[9];
};

/*
 * msgbuf : output 출력에 관한 정보를 담고 있다.
 * long type : FND, FND_WITH_BASE, DOT, LED
 * unsigned char text[10] : text lcd 출력을 위한 정보 또는 dot matrix 출력을 위한 정보를 담고 있다.
 * int num : fnd 출력을 위한 정보 또는 dot matrix 출력을 위한 정보 또는 led 출력을 위한 정보를 담고 있다.
 * int base : type이 FND_WITH_BASE일 경우 base정보를 담고 있다.
 */
struct msgbuf{
	long type;
	unsigned char text[10];
	int num;
	int base;
};

/*
 * eventbuf : event 버튼에 대한 메세지 전달을 위한 structure
 * long type : 항상 EVENT
 * int n : event 버튼에 대한 정보를 담고 있다.
 */
struct eventbuf{
	long type;
	int n;
};

/* function definition */
void change_mode();
void input_process();
void fnd_out(int num, int base);
void output_process();
void receive_msg();
void dot_out(int mode);
void text_out(const char*);
void dot_draw(unsigned char*);
void snd_msg();
void led_out(char n);

/* global variables */
int mode; //현재 모드 (CLOCK_MODE, COUNTER_MODE, TEXT_MODE, DRAW_MODE)
pid_t pid_in; //input process id
pid_t pid_out; //output process id
pthread_t p_thread[5];