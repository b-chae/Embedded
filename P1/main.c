#include "header.h"

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
			
			/* initializaiton when mode changed */
			if(mode == CLOCK_MODE){
				
			}
			else if(mode == COUNTER_MODE){
				
			}
			else if(mode == TEXT_MODE){
				
			}
		}
	}
}

void input_process(){
	key_t key1, key2, key3;
	struct switbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	buf.type = 1;
	buf.n = 0;
	memset(buf.value, 0, sizeof(buf.value));
	
	int i;
	int dev;
	
	dev = open(SWITCH_DEVICE, O_RDWR);
	unsigned char push_sw_buff[9];
	memset(push_sw_buff, 0, sizeof(push_sw_buff));	

	if(dev < 0){
		printf("Device open error\n");
		close(dev);
		return;
	}
	
	while(1){
		buf.n = 0;
		
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
			else{
				buf.value[i] = 0;
			}
		}
		
		while(push_sw_buff[0] == 1 || push_sw_buff[1] == 1 || push_sw_buff[2] == 1 || push_sw_buff[3] == 1
		|| push_sw_buff[4] == 1 || push_sw_buff[5] == 1 || push_sw_buff[6] == 1 || push_sw_buff[7] == 1 || push_sw_buff[8] == 1)
			read(dev, &push_sw_buff, sizeof(push_sw_buff));
		
		if(msgsnd(key1, (void*)&buf, sizeof(buf) - sizeof(long), IPC_NOWAIT) == -1){
			printf("msgsnd error\n");
			exit(0);
		}
		
		printf("send switch message %d switches pressed\n", buf.n);
		for(i=0; i<9; i++)
			printf("%d ", buf.value[i]);
		printf("\n");
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
	
	key_t key1, key2, key3;
	struct msgbuf buf;
	key2 = msgget((key_t)1002, IPC_CREAT|0666);

	while(1){
		if(msgrcv(key2, (void*)&buf, sizeof(buf) - sizeof(long), 0, MSG_NOERROR) == -1){
			printf("msgrcv error\n");
			exit(0);
		}
		else{
			printf("message received %d %d %s\n", buf.type, buf.num, buf.text);
			if(buf.type == FND){
				if(buf.type == 10){
						fnd_out(buf.num, 10);
				}
			}
			else if(buf.type == FND_WITH_BASE){
				fnd_out(buf.num, buf.base);
			}
		}
	}
	
}

void recieve_msg(){
	
	key_t key1, key2, key3;
	struct switbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	hour = tm.tm_hour;
	minuit = tm.tm_hour;
	
	int previous_hour, previous_minuit;
	fnd_out(hour*100 + minuit, 10);
	printf("current time %d %d\n", hour, minuit);
	
	int flag = 0;
	
	int text_count = 0;
	char text_buf[8];
	memset(text_buf, 0, sizeof(text_buf));
	char previous_char = ' ';
	char tmp;
	int i;
	strcpy(text_buf, "        ");
	
	while(1){
		msgrcv(key1, (void*)&buf, sizeof(buf) - sizeof(long), 1, 0);
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
			memset(buf2.text, 0, sizeof(buf2.text));
			strcpy(buf2.text, "");
			buf2.type = FND;
			buf2.num = hour*100 + minuit;
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
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
			printf("counter_number changed %d\n", counter_number);
			struct msgbuf buf2;
			memset(buf2.text, 0, sizeof(buf2.text));
			strcpy(buf2.text, "");
			buf2.type = FND_WITH_BASE;
			buf2.num = counter_number;
			buf2.base = counter_base;
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			printf("key2 sent \n");
		}
		else if(mode == TEXT_MODE){
			if(buf.n == 1){
				text_count++;
				
				int change_all = 0;
				if(buf.value[0] == 1){// .QZ
					if(previous_char == '.'){
						tmp = 'Q'; text_buf[7] = tmp;
					}
					else if(previous_char == 'Q'){
						tmp = 'Z'; text_buf[7] = tmp;
					}
					else if(previous_char == 'Z'){
						tmp = '.'; text_buf[7] = tmp;
					}
					else{
						tmp = '.'; change_all = 1;
					}
				}
				else if(buf.value[1] == 1){ //ABC
					if(previous_char == 'A'){
						tmp = 'B'; text_buf[7] = tmp;
					}
					else if(previous_char == 'B'){
						tmp = 'C'; text_buf[7] = tmp;
					}
					else if(previous_char == 'C'){
						tmp = 'A'; text_buf[7] = tmp;
					}
					else{
						tmp = 'A'; change_all = 1;
					}
				}
				else if(buf.value[2] == 1){ //DEF
					if(previous_char == 'D'){
						tmp = 'E'; text_buf[7] = tmp;
					}
					else if(previous_char == 'E'){
						tmp = 'F'; text_buf[7] = tmp;
					}
					else if(previous_char == 'F'){
						tmp = 'D'; text_buf[7] = tmp;
					}
					else{
						tmp = 'D'; change_all = 1;
					}
				}
				else if(buf.value[3] == 1){ //GHI
					if(previous_char == 'G'){
						tmp = 'H'; text_buf[7] = tmp;
					}
					else if(previous_char == 'H'){
						tmp = 'I'; text_buf[7] = tmp;
					}
					else if(previous_char == 'I'){
						tmp = 'G'; text_buf[7] = tmp;
					}
					else{
						tmp = 'G'; change_all = 1;
					}
				}
				else if(buf.value[4] == 1){ //JKL
					if(previous_char == 'J'){
						tmp = 'K'; text_buf[7] = tmp;
					}
					else if(previous_char == 'K'){
						tmp = 'L'; text_buf[7] = tmp;
					}
					else if(previous_char == 'L'){
						tmp = 'J'; text_buf[7] = tmp;
					}
					else{
						tmp = 'J'; change_all = 1;
					}
				}
				else if(buf.value[5] == 1){ //MNO
					if(previous_char == 'M'){
						tmp = 'N'; text_buf[7] = tmp;
					}
					else if(previous_char == 'N'){
						tmp = 'O'; text_buf[7] = tmp;
					}
					else if(previous_char == 'O'){
						tmp = 'M'; text_buf[7] = tmp;
					}
					else{
						tmp = 'M'; change_all = 1;
					}
				}
				else if(buf.value[6] == 1){ //PRS
					if(previous_char == 'P'){
						tmp = 'R'; text_buf[7] = tmp;
					}
					else if(previous_char == 'R'){
						tmp = 'S'; text_buf[7] = tmp;
					}
					else if(previous_char == 'S'){
						tmp = 'P'; text_buf[7] = tmp;
					}
					else{
						tmp = 'P'; change_all = 1;
					}
				}
				else if(buf.value[7] == 1){ //TUV
					if(previous_char == 'T'){
						tmp = 'U'; text_buf[7] = tmp;
					}
					else if(previous_char == 'U'){
						tmp = 'V'; text_buf[7] = tmp;
					}
					else if(previous_char == 'V'){
						tmp = 'T'; text_buf[7] = tmp;
					}
					else{
						tmp = 'T'; change_all = 1;
					}
				}
				else if(buf.value[8] == 1){ //WXY
					if(previous_char == 'W'){
						tmp = 'X'; text_buf[7] = tmp;
					}
					else if(previous_char == 'X'){
						tmp = 'Y'; text_buf[7] = tmp;
					}
					else if(previous_char == 'Y'){
						tmp = 'W'; text_buf[7] = tmp;
					}
					else{
						tmp = 'W'; change_all = 1;
					}
				}
				
				previous_char = tmp;
				
				if(change_all == 1){
					for(i=0; i<7; i++){
					text_buf[i] = text_buf[i+1];							}
					text_buf[7] = tmp;
				}
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
			memset(buf2.text, 0, sizeof(buf2.text));
			strcpy(buf2.text, text_buf);
			buf2.type = FND;
			buf2.num = text_count;
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
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
			r_value = pthread_create(&p_thread[0], NULL, change_mode, NULL);
			r_value = pthread_create(&p_thread[1], NULL, recieve_msg, NULL);
			pthread_join(p_thread[0], (void**)NULL);
			pthread_join(p_thread[1], (void**)NULL);
		}
		//main process
		
	}
	return 0;
}
