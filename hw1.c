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

#define BUFF_SIZE 64
#define KEY_RELEASE 0
#define KEY_PRESS 1

#define MAX_DIGIT 4
#define FND_DEVICE "/dev/fpga_fnd"
#define SWITCH_DEVICE "/dev/fpga_push_switch"

struct switbuf{
	int n;
	int value[10];
}


struct msgbuf{
	int type;
	char text[50];
	int num;
	int base;
};

void fnd_out(int num, int base);

int mode = 0;
pid_t pid_in;
pid_t pid_out;
pthread_t p_thread[3];
int r_value;

int counter_base = 2;
int counter_number = 0;

void change_mode(){
	
	struct input_event ev[BUFF_SIZE];
	int fd, rd, value, size = sizeof(struct input_event);
	
	char* device = "/dev/input/event0";
	if((fd = open(device, O_RDONLY)) == -1){
		printf("%s is not a valid device.\n", device);
	}


	while(1){
		
		if((rd = read(fd, ev, size*BUFF_SIZE)) < size){
			printf("read()");
			return 0;
		}
		
		value = ev[0].value;
		
		if(value == KEY_PRESS){
			if(ev[0].code == 115){ //volume up
				mode = (mode + 1) % 4;
				printf("mode changed : %d\n", mode);
			}
			else if(ev[0].code == 114){ //volume down
				mode--;
				if(mode < 0) mode = mode + 4;
				printf("mode changed : %d\n", mode);
			}
			
			
		}
	}
}

void input_process(){
	key_t key1, key2, key3;
	struct switbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	buf.n = 0;
	memset(buf.value, 0, sizeof(buf.value);
	
	int i;
	int dev;
	
	for(i=0; i<9; i++){
		buf.value[i] = 0;
	}
	
	dev = open(SWITCH_DEVICE, O_RDWR);
	unsigned char push_sw_buff[9];
	
	if(dev < 0){
		printf("Device open error\n");
		close(dev);
		return;
	}
	
	while(1){
		
		while(push_sw_buff[0] == 0 && push_sw_buff[1] == 0 && push_sw_buff[2] == 0 && push_sw_buff[3] == 0 
		&& push_sw_buff[4] == 0 && push_sw_buff[5] == 0 && push_sw_buff[6] == 0 && push_sw_buff[7] == 0 
		&& push_sw_buff[8] == 0)
			read(dev, &push_sw_buff, sizeof(push_sw_buff));
		
		for(i=0; i<9; i++){
			if(push_sw_buff[i] == 1)
			{
				buf.n++;
				buf.value[i] = 1;
			}
		}
		
		while(push_sw_buff[0] == 1 || push_sw_buff[1] == 1 || push_sw_buff[2] == 1 || push_sw_buff[3] == 1
		|| push_sw_buff[4] == 1 || push_sw_buff[5] == 1 || push_sw_buff[6] == 1 || push_sw_buff[7] == 1 || push_sw_buff[8] == 1)
			read(dev, &push_sw_buff, sizeof(push_sw_buff));
		
		if(msgsnd(key1, (void*)&buf, sizeof(buf), IPC_NOWAIT) == -1){
			printf("msgsnd error\n");
			exit(0);
		}
		
		printf("send switch message %d switches pressed\n", buf.n);
	}
	close(dev);
}

void fnd_out(int num, int base){
	int dev;
	unsigned char data[4];
	unsigned char retval;

	memset(data, 0, sizeof(data));
	
	data[0] = num/(base*base*base);
	data[1] = num/(base*base)%base;
	data[2] = num%(base*base)/base;
	data[3] = num%base;

	dev = open(FND_DEVICE, O_RDWR);
	if(dev < 0){
		printf("Device open error %s\n", FND_DEVICE);
		exit(1);
	}
	retval = write(dev, &data, 4);
	
	memset(data, 0, sizeof(data));
	
	close(dev);
}

void output_process(){
	
	int msgtype = 1;
	key_t key1, key2, key3;
	struct msgbuf buf;
	key2 = msgget((key_t)1002, IPC_CREAT|0666);

	while(1){
		if(msgrcv(key2, (void*)&buf, sizeof(buf), 0, 0) == -1)
			printf("error\n");
		printf("message received %d %d\n", buf.type, buf.num);
		if(buf.type == 10){
				fnd_out(buf.num, 10);
		}
	}
	
}

void recieve_msg(){
	
	int msgtype;
	key_t key1, key2, key3;
	struct switbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	
	int hour = 1;
	int minuit = 1;
	
	int flag = 0;
	
	while(1){
		msgrcv(key1, (void*)&buf, sizeof(buf), msgtype, 0);
		printf("key1 received\n");
		if(mode == CLOCK_MODE){
			if(buf.n == 1 && buf.value[0] == 1){
				if(flag == 0){
					flag = 1;
					previous_hour = hour;
					previous_minuit = minuit;
				}else{
					flag = 0;	
				}
			}
			else if(buf.n == 1 && buf.value[1] == 1){
				hour = previous_hour;
				minuit = previous_minuit;
			}
			else if(buf.n == 1 && buf.value[2] == 1){
				hour = (hour + 1) % 24;
			}
			else if(buf.n == 1 && buf.value[3] == 1){
				minuit = (minuit + 1) % 60;
			}
			
			struct msgbuf buf2;
			buf2.type = FND;
			buf2.num = hour*100 + minuit;
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			msgsnd(key2, (void*)&buf2, sizeof(buf2), IPC_NOWAIT);
		}
	}
}

int main(int argc, char *argv[]){
	
	
	pid_in = fork();
	if(pid_in == 0){//child process : receive input
		input_process();
	}else{//parent
		pid_out = fork();
		if(pid_out == 0){
			output_process();
		}
		else{
			
		}
		//main process
		
		pthread_create(&p_thread[0], NULL, change_mode, NULL);
		pthread_create(&p_thread[1], NULL, recieve_msg, NULL);
		pthread_join(p_thread[0], (void**)NULL);
		pthread_join(p_thread[1], (void**)NULL);
		
	}
	return 0;
}
