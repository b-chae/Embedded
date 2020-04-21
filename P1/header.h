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

#define BUFF_SIZE 64
#define KEY_RELEASE 0
#define KEY_PRESS 1

#define MAX_DIGIT 4
#define FND_DEVICE "/dev/fpga_fnd"
#define SWITCH_DEVICE "/dev/fpga_push_switch"

#define CLOCK_MODE 0
#define COUNTER_MODE 1
#define TEXT_MODE 2

#define FND 10
#define FND_WITH_BASE 11

/*struct definition*/
struct switbuf{
	long type;
	int n;
	unsigned char value[9];
};

struct msgbuf{
	long type;
	char text[50];
	int num;
	int base;
};

/* function definition */
void change_mode();
void input_process();
void fnd_out(int num, int base);
void output_process();
void receive_msg();

/* global variables */
int mode = 0;
pid_t pid_in;
pid_t pid_out;
pthread_t p_thread[3];
int r_value;

int counter_base = 2;
int counter_number = 0;

int hour;
int minuit;
