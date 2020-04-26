#include "header.h"

unsigned char draw_board[10];

int text_count;
char text_buf[8];
int draw_count;
int flag;
char previous_char;

void receive_msg(){
	
	key_t key1, key2, key3;
	struct switbuf buf;
	key1 = msgget((key_t)1001, IPC_CREAT|0666);
	
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	
	hour = tm->tm_hour;
	minuit = tm->tm_min;
	
	flag = 0;
	memset(text_buf, 0, sizeof(text_buf));
	char tmp;
	int i;
	
	memset(draw_board, 0, sizeof(draw_board));
	
	while(1){
		msgrcv(key1, (void*)&buf, sizeof(buf) - sizeof(long), SWITCH, 0);
		printf("key1 received\n");
		if(mode == CLOCK_MODE){
			if(buf.n == 1 && buf.value[0] == 1){
				if(flag == 0){
					flag = 1;
				}else{
					flag = 0;

					struct msgbuf buf2;
					memset(buf2.text, 0, sizeof(buf2.text));
					buf2.type = LED;
					buf2.num = 128;
					key2 = msgget((key_t)1002, IPC_CREAT|0666);
					if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
						printf("key 2 msgsnd error\n");
						exit(0);
					}					
				}
			}
			else if(buf.n == 1 && buf.value[1] == 1 && flag == 1){
				hour = tm->tm_hour;
				minuit = tm->tm_min;
			}
			else if(buf.n == 1 && buf.value[2] == 1 && flag == 1){
				hour = ( hour + 1 ) % 24;
			}
			else if(buf.n == 1 && buf.value[3] == 1 && flag == 1){
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
			
			buf2.type = LED;
			if(counter_base == 2) buf2.num = 128;
			else if(counter_base == 10) buf2.num = 64;
			else if(counter_base == 8) buf2.num = 32;
			else if(counter_base == 4) buf2.num = 16;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
		}
		else if(mode == TEXT_MODE){
			if(buf.n == 1){
				text_count = (text_count + 1)%10000;
				
				if(text_mode == 0){
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
						text_buf[i] = text_buf[i+1];}
						text_buf[7] = tmp;
					}
					
				}
				else{
					for(i=0; i<9 ;i++){
						if(buf.value[i] == 1){
							tmp = i + 1 + '0';
						}
					}
					
					for(i=0; i<7; i++){
						text_buf[i] = text_buf[i+1];
					}
					text_buf[7] = tmp;
					previous_char = tmp;
				}
			}
			else if(buf.n == 2){
				text_count = (text_count + 1)%10000;
				
				if(buf.value[1] == 1 && buf.value[2] == 1){
					for(i=0; i<8; i++)
						text_buf[i] = ' ';
					previous_char = ' ';
				}
				else if(buf.value[4] == 1 && buf.value[5] == 1){//change mode to alphabet or number
					if(text_mode == 0) text_mode = 1; //change to number mode
					else text_mode = 0; //change to alphabet mode
					previous_char = ' ';
					//DOT MATRIX 출력한다. alphabet mode? number mode?
					struct msgbuf buf2;
					memset(buf2.text, 0, sizeof(buf2.text));
					strcpy(buf2.text, "");
					buf2.type = DOT;
					buf2.num = text_mode;
					key2 = msgget((key_t)1002, IPC_CREAT|0666);
					if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
						printf("key 2 msgsnd error\n");
						exit(0);
					}
					printf("dot key2 sent \n");
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
		else if(mode == DRAW_MODE){
			if(buf.n == 1){
				
				draw_count = (draw_count + 1)%10000;
				
				if(buf.value[0] == 1){ //reset
					for(i=0; i<10; i++){
						draw_board[i] = 0;
					}
					cursorX = 0;
					cursorY = 6;
					isCursor = 1;
				}
				else if(buf.value[1] == 1){
					if(cursorX > 0) cursorX--;
				}
				else if(buf.value[2] == 1){ //cursor
					if(isCursor == 1) isCursor = 0;
					else isCursor = 1;
				}
				else if(buf.value[3] == 1){
					if(cursorY < 6) cursorY++;
				}
				else if(buf.value[4] == 1){
					switch(cursorY){
						case 6: draw_board[cursorX] = draw_board[cursorX] | 0b01000000; break;
						case 5: draw_board[cursorX] = draw_board[cursorX] | 0b00100000; break;
						case 4: draw_board[cursorX] = draw_board[cursorX] | 0b00010000; break;
						case 3: draw_board[cursorX] = draw_board[cursorX] | 0b00001000; break;
						case 2: draw_board[cursorX] = draw_board[cursorX] | 0b00000100; break;
						case 1: draw_board[cursorX] = draw_board[cursorX] | 0b00000010; break;
						case 0: draw_board[cursorX] = draw_board[cursorX] | 0b00000001; break;
					}
				}
				else if(buf.value[5] == 1){
					if(cursorY > 0) cursorY--;
				}
				else if(buf.value[6] == 1){
					for(i=0; i<10; i++){
						draw_board[i] = 0;
					}
				}
				else if(buf.value[7] == 1){
					if(cursorX < 9) cursorX++;
				}
				else if(buf.value[8] == 1){
					for(i=0; i<10; i++){
						draw_board[i] = ~draw_board[i] % 128;
					}
				}
				
				struct msgbuf buf2;
				memset(buf2.text, 0, sizeof(buf2.text));
				for(i=0; i<10; i++)
					buf2.text[i] = draw_board[i];
				buf2.type = DOT;
				buf2.num = 2;
				key2 = msgget((key_t)1002, IPC_CREAT|0666);
				if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
					printf("key 2 msgsnd error\n");
					exit(0);
				}
				printf("dot key2 sent \n");
				
				strcpy(buf2.text, "");
				buf2.type = FND;
				buf2.num = draw_count;
				if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
					printf("key 2 msgsnd error\n");
					exit(0);
				}
				printf("draw_count key2 sent \n");

				for(i=0; i<10;i++)
					printf("[%d]",draw_board[i]);
				printf("\n");
			}
		}
	}
}

void change_mode(){

	struct eventbuf buf;
	struct msgbuf buf2;
	key_t key = msgget((key_t)1003, IPC_CREAT|0666);
	key_t key2 = msgget((key_t)1002, IPC_CREAT|0666);
	memset(buf2.text, 0, sizeof(buf2.text));
	int i;
	
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	//led 초기화
	buf2.type = LED;
	buf2.num = 128;
	if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
		printf("key 2 msgsnd error\n");
		exit(0);
	}

	while(1){
		
		msgrcv(key, (void*)&buf, sizeof(buf) - sizeof(long), EVENT, 0);
		
		if(buf.n == 115){ //volume up
			mode = (mode + 1) % 4;
			printf("mode changed : %d\n", mode);
		}
		else if(buf.n == 114){ //volume down
			mode--;
			if(mode < 0) mode = mode + 4;
			printf("mode changed : %d\n", mode);
		}
		else if(buf.n == 116){
			//DOT MATRIX 초기화
			buf2.type = DOT;
			buf2.num = -1;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//text fnd 초기화
			buf2.type = FND;
			buf2.num = 0;
			strcpy(buf2.text, "        ");
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//led 초기화
			buf2.type = LED;
			buf2.num = 0;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			sleep(1);
			kill(pid_in, SIGINT);
			kill(pid_out, SIGINT);
			printf("Good bye\n");
			exit(0);
		}
		
		/* initializaiton when mode changed */
		if(mode == CLOCK_MODE){
			isCursor = 0;
			flag = 0;
			//DOT MATRIX 초기화
			buf2.type = DOT;
			buf2.num = -1;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//text fnd 초기화
			hour = tm->tm_hour;
			minuit=tm->tm_min;
			buf2.type = FND;
			buf2.num = hour*100 + minuit;
			strcpy(buf2.text, "        ");
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//led 초기화
			buf2.type = LED;
			buf2.num = 128;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
		}
		else if(mode == COUNTER_MODE){
			isCursor = 0;
			//DOT MATRIX 초기화
			buf2.type = DOT;
			buf2.num = -1;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//text fnd 초기화
			buf2.type = FND;
			buf2.num = 0;
			strcpy(buf2.text, "        ");
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			counter_number = 0;
			counter_base = 10;
			//led 초기화
			buf2.type = LED;
			buf2.num = 64;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
		}
		else if(mode == TEXT_MODE){
			text_mode = 0;
			previous_char = ' ';
			text_count = 0;
			strcpy(text_buf, "        ");
			isCursor = 0;
			//DOT MATRIX 초기화
			buf2.type = DOT;
			buf2.num = 0;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//text fnd 초기화
			buf2.type = FND;
			buf2.num = 0;
			strcpy(buf2.text, "        ");
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//led 초기화
			buf2.type = LED;
			buf2.num = 0;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
		}
		else if(mode == DRAW_MODE){
			isCursor  = 1;
			cursorX = 0;
			cursorY = 6;
			draw_count = 0;
			for(i=0; i<10; i++){
				draw_board[i] = 0;
			}
			//DOT MATRIX 초기화
			buf2.type = DOT;
			buf2.num = 0;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//text fnd 초기화
			buf2.type = FND;
			buf2.num = 0;
			strcpy(buf2.text, "        ");
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//led 초기화
			buf2.type = LED;
			buf2.num = 0;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
		}

	}
}

void snd_msg(){
	
	int i;
	key_t key2;

	while(1){
		if(isCursor == 1){

			int tmpX = cursorX;
			int tmpY = cursorY;
			char tmp_board[10];
			memset(tmp_board, 0, sizeof(tmp_board));
			for(i=0; i<10; i++){
				tmp_board[i] = draw_board[i];
			}

			struct msgbuf buf2;
			memset(buf2.text, 0, sizeof(buf2.text));
			key2 = msgget((key_t)1002, IPC_CREAT|0666);

			switch(tmpY){
				case 6: tmp_board[tmpX] = tmp_board[tmpX] | 0b01000000; break;
				case 5: tmp_board[tmpX] = tmp_board[tmpX] | 0b00100000; break;
				case 4: tmp_board[tmpX] = tmp_board[tmpX] | 0b00010000; break;
				case 3: tmp_board[tmpX] = tmp_board[tmpX] | 0b00001000; break;
				case 2: tmp_board[tmpX] = tmp_board[tmpX] | 0b00000100; break;
				case 1: tmp_board[tmpX] = tmp_board[tmpX] | 0b00000010; break;
				case 0: tmp_board[tmpX] = tmp_board[tmpX] | 0b00000001; break;
			}
			
			buf2.type = DOT;
			buf2.num = 2;
			
			for(i=0; i<10; i++){
				buf2.text[i] = tmp_board[i];
			}

			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			
			sleep(1);
			
			for(i=0; i<10; i++){
				tmp_board[i] = draw_board[i];
			}
			
			switch(tmpY){
				case 6: tmp_board[tmpX] = tmp_board[tmpX] & 0b10111111; break;
				case 5: tmp_board[tmpX] = tmp_board[tmpX] & 0b11011111; break;
				case 4: tmp_board[tmpX] = tmp_board[tmpX] & 0b11101111; break;
				case 3: tmp_board[tmpX] = tmp_board[tmpX] & 0b11110111; break;
				case 2: tmp_board[tmpX] = tmp_board[tmpX] & 0b11111011; break;
				case 1: tmp_board[tmpX] = tmp_board[tmpX] & 0b11111101; break;
				case 0: tmp_board[tmpX] = tmp_board[tmpX] & 0b11111110; break;
			}
			
			buf2.type = DOT;
			buf2.num = 2;
			for(i=0; i<10; i++){
				buf2.text[i] = tmp_board[i];
			}
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
		}
		if(flag == 1){
			struct msgbuf buf2;
			memset(buf2.text, 0, sizeof(buf2.text));
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			
			buf2.type = LED;
			buf2.num = 32;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			
			sleep(1);
			if(flag == 1){
				buf2.num = 16;
				if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
					printf("key 2 msgsnd error\n");
					exit(0);
				}
			}
		}
		sleep(1);
	}
}

int main(int argc, char *argv[]){
	
	mode = 0;
	
	pid_in = fork();
	if(pid_in == 0){//child process : receive input
		input_process();
	}else{//parent
		pid_out = fork();
		if(pid_out == 0){
			output_process();
		}
		else{
			r_value = pthread_create(&p_thread[0], NULL, change_mode, (void *)NULL);
			r_value = pthread_create(&p_thread[1], NULL, receive_msg, (void *)NULL);
			r_value = pthread_create(&p_thread[2], NULL, snd_msg, (void *)NULL);
			pthread_join(p_thread[0], (void**)NULL);
			pthread_join(p_thread[1], (void**)NULL);
			pthread_join(p_thread[2], (void**)NULL);
		}
		//main process
		
	}
	return 0;
}
