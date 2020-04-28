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
#include <sys/shm.h>

#define BUFF_SIZE 64
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

int mid1, mid2, mid3;
char* maddr1; char* maddr2; char* maddr3;