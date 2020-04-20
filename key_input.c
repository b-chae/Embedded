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

struct msgbuf{
	int type;
	char text[50];
	int num;
};

int mode = 0;
pid_t pid_in;
pid_t pid_out;
pthread_t p_thread[3];

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
	struct msgbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	memset(buf.text, 0, sizeof(buf.text));
	strcpy(buf.text, "");
	buf.type = -1;
	buf.num = -1;
	
	int i;
	int dev;
	
	dev = open(SWITCH_DEVICE, O_RDWR);
	unsigned char push_sw_buff[9];
	
	if(dev < 0){
		printf("Device open error\n");
		close(dev);
		return;
	}
	
	while(1){
		while(push_sw_buff[0] == 0 && push_sw_buff[1] == 0 && push_sw_buff[2] == 0 && push_sw_buff[3] == 0)
			read(dev, &push_sw_buff, sizeof(push_sw_buff));
		
		for(i=0; i<4; i++){
			if(push_sw_buff[i] == 1)
			{
				buf.type = 1;
				buf.num = i;
			}
		}
		
		while(push_sw_buff[0] == 1 || push_sw_buff[1] == 1 || push_sw_buff[2] == 1 || push_sw_buff[3] == 1)
			read(dev, &push_sw_buff, sizeof(push_sw_buff));
		
		if(msgsnd(key1, (void*)&buf, sizeof(buf), IPC_NOWAIT) == -1){
			printf("msgsnd error\n");
			exit(0);
		}
		
		printf("send switch message\n");
	}
	close(dev);
	//memset(data, 0, sizeof(data));
		
/*	while(1){
	if(mode == 0){
	
		close(dev);
		
		memset(data, 0, sizeof(data));
		
		data[0] = hour/10;
		data[1] = hour%10;
		data[2] = minuit/10;
		data[3] = minuit%10;

		dev = open(FND_DEVICE, O_RDWR);
		if(dev < 0){
			printf("Device open error %s\n", FND_DEVICE);
			exit(1);
		}
		retval = write(dev, &data, 4);
		
		memset(data, 0, sizeof(data));
		
		close(dev);
	}*/
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
		
		int dev;
		unsigned char data[4];
		unsigned char retval;

		memset(data, 0, sizeof(data));
		
		data[0] = buf.num/1000;
		data[1] = buf.num/100%10;
		data[2] = buf.num%100/10;
		data[3] = buf.num%10;

		dev = open(FND_DEVICE, O_RDWR);
		if(dev < 0){
			printf("Device open error %s\n", FND_DEVICE);
			exit(1);
		}
		retval = write(dev, &data, 4);
		
		memset(data, 0, sizeof(data));
		
		close(dev);
		}
	}
	
}

void recieve_msg(){
	
	int msgtype;
	key_t key1, key2, key3;
	struct msgbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	
	int hour = 1;
	int minuit = 1;
	
	while(1){
		msgrcv(key1, (void*)&buf, sizeof(buf), msgtype, 0);
		if(buf.type == 1 && mode == 0){
			if(buf.num == 0){
				
			}
			else if(buf.num == 1){
				
			}
			else if(buf.num == 2){
				hour = (hour + 1) % 24;
			}
			else{
				minuit = (minuit + 1) % 60;
			}
			
			struct msgbuf buf2;
			buf2.type = 10;
			buf2.num = hour*100 + minuit;
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			msgsnd(key2, (void*)&buf2, sizeof(buf), IPC_NOWAIT);
			printf("send to output process %d %d\n", buf2.type, buf2.num);
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
