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
#define COUNTER_MODE 1
#define TEXT_MODE 2

#define FND 10
#define FND_WITH_BASE 11

struct switbuf{
	int n;
	unsigned char value[9];
};

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
			return;
		}

		printf("event button pressed\n");
		
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
			
			if(mode == COUNTER_MODE){
				//initialize
				//fnd_out(counter_number, counter_base);
			}
		}
	}
}

void input_process(){
	key_t key1, key2, key3;
	struct switbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	memset(buf.value, 0, sizeof(buf.value));
	buf.n = 0;
	
	int i;
	int dev;
	
	dev = open(SWITCH_DEVICE, O_RDWR);
	unsigned char push_sw_buff[9];
	memset(push_sw_buff, 0, sizeof(push_sw_buff));
	for(i = 0; i<9; i++){
		push_sw_buff[i] = 0;
	}	

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
			printf("%d ",push_sw_buff[i]);
		}
		
		while(push_sw_buff[0] == 1 || push_sw_buff[1] == 1 || push_sw_buff[2] == 1 || push_sw_buff[3] == 1
		|| push_sw_buff[4] == 1 || push_sw_buff[5] == 1 || push_sw_buff[6] == 1 || push_sw_buff[7] == 1 || push_sw_buff[8] == 1)
			read(dev, &push_sw_buff, sizeof(push_sw_buff));
		
		if(msgsnd(key1, (void*)&buf, sizeof(buf), IPC_NOWAIT) == -1){
			printf("key 1 msgsnd error\n");
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
		printf("in output process : key2 message received\n");
		if(buf.type == FND){
			fnd_out(buf.num, 10);
		}
		else if(buf.type == FND_WITH_BASE){
			fnd_out(buf.num, buf.base);
		}
	}
	
}

void recieve_msg(){
	
	key_t key1, key2, key3;
	struct switbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	int hour = tm.tm_hour;
	int minuit = tm.tm_hour;
	int previous_hour;
	int previous_minuit;
	
	fnd_out(hour*100 + minuit, 10);
	
	int flag = 0;
	
	int text_count = 0;
	char text_buf[8];
	memset(text_buf, ' ', sizeof(text_buf));
	int text_mode = 0; //0 : alphabet mode, 1 : number mode
	char previous_char = ' ';
	char tmp;
	int i;
	
	while(1){
		msgrcv(key1, (void*)&buf, sizeof(buf), 0, 0); //key input received
		printf(" in recieve msg , key1 received\n");
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
			printf("line 250 message send");
		}
		else if(mode == COUNTER_MODE){
			if(buf.n == 1 && buf.value[0] == 1){
				if(counter_base == 2) counter_base = 10;
				else if(counter_base == 10) counter_base = 8;
				else if(counter_base == 8) counter_base = 4;
				else if(counter_base == 4) counter_base = 2;
			}
			else if(buf.n == 1 && buf.value[1] == 1){
				counter_number += counter_base*counter_base;
			}
			else if(buf.n == 1 && buf.value[2] == 1){
				counter_number += counter_base;
			}
			else if(buf.n == 1 && buf.value[3] == 1){
				counter_number += 1;
			}
			
			counter_number = counter_number % (counter_base*counter_base*counter_base);
			struct msgbuf buf2;
			buf2.type = FND_WITH_BASE;
			buf2.num = counter_number;
			buf2.base = counter_base;
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			msgsnd(key2, (void*)&buf2, sizeof(buf2), IPC_NOWAIT);
		}
		else if(mode == TEXT_MODE){
			if(buf.n == 1){
				text_count++;
				
				if(buf.value[0] == 1){// .QZ
					if(previous_char == '.'){
						tmp = 'Q';
					}
					else if(previous_char == 'Q'){
						tmp = 'Z';
					}
					else if(previous_char == 'Z'){
						tmp = '.';
					}
					else{
						tmp = '.';
					}
				}
				else if(buf.value[1] == 1){ //ABC
					if(previous_char == 'A'){
						tmp = 'B';
					}
					else if(previous_char == 'B'){
						tmp = 'C';
					}
					else if(previous_char == 'C'){
						tmp = 'A';
					}
					else{
						tmp = 'A';
					}
				}
				else if(buf.value[2] == 1){ //DEF
					if(previous_char == 'D'){
						tmp = 'E';
					}
					else if(previous_char == 'E'){
						tmp = 'F';
					}
					else if(previous_char == 'F'){
						tmp = 'D';
					}
					else{
						tmp = 'D';
					}
				}
				else if(buf.value[3] == 1){ //GHI
					if(previous_char == 'G'){
						tmp = 'H';
					}
					else if(previous_char == 'H'){
						tmp = 'I';
					}
					else if(previous_char == 'I'){
						tmp = 'G';
					}
					else{
						tmp = 'G';
					}
				}
				else if(buf.value[4] == 1){ //JKL
					if(previous_char == 'J'){
						tmp = 'K';
					}
					else if(previous_char == 'K'){
						tmp = 'L';
					}
					else if(previous_char == 'L'){
						tmp = 'J';
					}
					else{
						tmp = 'J';
					}
				}
				else if(buf.value[5] == 1){ //MNO
					if(previous_char == 'M'){
						tmp = 'N';
					}
					else if(previous_char == 'N'){
						tmp = 'O';
					}
					else if(previous_char == 'O'){
						tmp = 'M';
					}
					else{
						tmp = 'M';
					}
				}
				else if(buf.value[6] == 1){ //PRS
					if(previous_char == 'P'){
						tmp = 'R';
					}
					else if(previous_char == 'R'){
						tmp = 'S';
					}
					else if(previous_char == 'S'){
						tmp = 'P';
					}
					else{
						tmp = 'P';
					}
				}
				else if(buf.value[7] == 1){ //TUV
					if(previous_char == 'T'){
						tmp = 'U';
					}
					else if(previous_char == 'U'){
						tmp = 'V';
					}
					else if(previous_char == 'V'){
						tmp = 'T';
					}
					else{
						tmp = 'T';
					}
				}
				else if(buf.value[8] == 1){ //WXY
					if(previous_char == 'W'){
						tmp = 'X';
					}
					else if(previous_char == 'X'){
						tmp = 'Y';
					}
					else if(previous_char == 'Y'){
						tmp = 'W';
					}
					else{
						tmp = 'W';
					}
				}
				for(i=0; i<7; i++){
				text_buf[i] = text_buf[i+1];							}
				text_buf[7] = tmp;
			}
			else if(buf.n == 2){
				text_count++;
				
				if(buf.value[1] == 1 && buf.value[2] == 1){
					for(i=0; i<8; i++)
						text_buf[i] = ' ';
					previous_char = ' ';
				}
				else if(buf.value[4] == 1 && buf.value[5] == 1){//change mode to alphabet or number
					if(text_mode == 0) text_mode = 1; //change to number mode
					else text_mode = 0; //change to alphabet mode
					previous_char = ' ';
				}
				else if(buf.value[7] == 1 && buf.value[8] == 1){//insert a blank at the end
					for(i=0; i<7; i++){
						text_buf[i] = text_buf[i+1];
					}
					text_buf[7] = ' ';
					previous_char = ' ';
				}
			}
			//TEXT LCD 출력한다.
			//DOT MATRIX 출력한다. alphabet mode? number mode?
			//text_count를 FND 출력한다.
			struct msgbuf buf2;
			buf2.type = FND;
			buf2.num = text_count;
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			msgsnd(key2, (void*)&buf2, sizeof(buf2), IPC_NOWAIT);
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
		//main process
		r_value = pthread_create(&p_thread[0], NULL, change_mode, NULL);
		r_value = pthread_create(&p_thread[1], NULL, recieve_msg, NULL);
		pthread_join(p_thread[0], (void**)NULL);
		pthread_join(p_thread[1], (void**)NULL);	
		}
	}
	return 0;
}
