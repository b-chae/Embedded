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
#include <signal.h>
 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>

#define BUFF_SIZE 64
#define KEY_RELEASE 0
#define KEY_PRESS 1

#define MAX_DIGIT 4
#define FND_DEVICE "/dev/fpga_fnd"
#define SWITCH_DEVICE "/dev/fpga_push_switch"

#define CLOCK_MODE 0

#define FND 10
#define SWITCH 1

struct msgbuf{
	int type;
	char text[50];
	int num;
}

int mode = 0;
pid_t pid_in;
pid_t pid_out;
pthread_t p_thread[3];
int r_value;

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
			return;
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
}

void fnd_out(int num){

	int dev;
	unsigned char data[4];
	unsigned char retval;

	memset(data, 0, sizeof(data));

	data[0] = num/1000;
	data[1] = num/100%10;
	data[2] = num%100/10;
	data[3] = num%10;

	dev = open(FND_DEVICE, O_RDWR);
	if(dev < 0){
		printf("Device open error %s\n", FND_DEVICE);
		exit(1);
	}
	retval = write(dev, &data, 4);
	printf("fnd set %d%d%d%d\n", data[0], data[1], data[2], data[3]);

	memset(data, 0, sizeof(data));

	close(dev);
}

void output_process(){

	key_t key1, key2, key3;
	struct msgbuf buf;
	key2 = msgget((key_t)1002, IPC_CREAT|0666);

	while(1){
		msgrcv(key2, (void*)&buf, sizeof(buf), 0, 0);
		if(buf.type == FND){
			fnd_out(buf.num);
		}
	}

}

void recieve_msg(){

	key_t key1, key2, key3;
	struct msgbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	int hour = tm.tm_hour;
	int minuit = tm.tm_hour;
	int previous_hour;
	int previous_minuit;

	fnd_out(hour*100 + minuit);

	int flag = 0;

	while(1){
		msgrcv(key1, (void*)&buf, sizeof(buf), 0, 0); //key input received
		if(buf.type == SWITCH && mode == CLOCK_MODE){
			if(buf.num == 0){
				if(flag == 0){
					flag = 1;
					previous_hour = hour;
					previous_minuit = minuit;
				}else{
					flag = 0;	
				}
			}
			else if(buf.num == 1){
				hour = previous_hour;
				minuit = previous_minuit;
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
		}
	}
}

int main(int argc, char *argv[]){


	pid_t pid_in = fork();
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

		r_value = pthread_create(&p_thread[0], NULL, change_mode, NULL);
		r_value = pthread_create(&p_thread[1], NULL, recieve_msg, NULL);
		pthread_join(p_thread[0], (void**)NULL);
		pthread_join(p_thread[1], (void**)NULL);

	}
	return 0;
} 
