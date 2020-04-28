#include "header.h"

/* CLOCK_MODE */
int flag; //flag = 1 if CLOCK_MODE에서 시간 수정 중일 경우, flag = 0 if CLOCK_MODE에서 시간 수정 안하고 있는 경우
int hour;
int minuit;
/* COUNTER_MODE */
int counter_base;
int counter_number;
/* TEXT_MODE */
int text_mode; //text_mode = 0 if TEXT_MODE에서 영어입력 중일 경우, text_mode = 1 if TEXT_MODE에서 숫자 입력중일 경우
int text_count; //버튼이 눌러진 횟수
char text_buf[8];
char previous_char;
/* DRAW_MODE */
unsigned char draw_board[10];
int isCursor; //1일 경우 커서 보이기, 0일 경우 커서 안보이기
int cursorX; int cursorY; //커서 X, Y좌표
int draw_count; //DRAW_MODE에서 버튼이 눌러진 횟수

/* input process에서 스위치 입력을 받은 경우 메세지를 처리하는 함수 */
void receive_msg(){
	
	int shmid1 = shmget((key_t)1001, 11, IPC_CREAT|0666);
	char* shmaddr1 = (char*)shmat(shmid1, (char*)NULL, 0);
	int shmid2 = shmget((key_t)1002, 14, IPC_CREAT|0666);
	char* shmaddr2 = (char*)shmat(shmid2, (char*)NULL, 0);
	
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);	
	flag = 0;
	memset(text_buf, 0, sizeof(text_buf));
	char tmp;
	int i, n;
	
	memset(draw_board, 0, sizeof(draw_board));
	
	while(1){
		/* 스위치 입력을 받는다 */
		if(*shmaddr1 == SWITCH){
			*shmaddr1 = '*';
			n = shmaddr1[1];
			if(mode == CLOCK_MODE){
				if(n == 1 && shmaddr1[2] == 1){ //1번 스위치, 시간 수정 모드 -> 시간 변경 끝, 시간 변경 끝 -> 시간 수정 모드
					if(flag == 0){ //시간 변경 끝 -> 시간 수정모드
						flag = 1;
					}else{ //시간 수정 모드 -> 시간 변경 끝, LED 1번에 불이 들어오도록 한다.
						flag = 0;

						shmaddr2[3] = '\0';
						shmaddr2[1] = 28;
						shmaddr2[2] = 1;
						shmaddr2[0] = LED;
						sleep(1);
					}
				} //2번 스위치, 시간 수정 모드일 경우 보드의 시간으로 reset한다.
				else if(n == 1 && shmaddr1[3] == 1 && flag == 1){
					hour = tm->tm_hour;
					minuit = tm->tm_min;
				} //3번 스위치, 시간 수정 모드일 경우 시간 + 1
				else if(n == 1 && shmaddr1[4] == 1 && flag == 1){
					hour = ( hour + 1 ) % 24;
				} //4번 스위치, 시간 수정 모드일 경우 분 + 1
				else if(n == 1 && shmaddr1[5] == 1 && flag == 1){
					minuit = (minuit + 1) % 60;
				}
				//시간이 바뀐 경우 FND를 hour*100 + minuit으로 설정한다. output process에 메세지 전달.
				shmaddr2[3] = '\0';
				shmaddr2[1] = minuit;
				shmaddr2[2] = hour;
				shmaddr2[0] = FND;
				sleep(1);
			}
			else if(mode == COUNTER_MODE){
				if(n == 1 && shmaddr1[2] == 1){ //1번 스위치, 진수를 2 -> 10 -> 8 -> 4로 바꾼다.
					if(counter_base == 2) counter_base = 10;
					else if(counter_base == 10) counter_base = 8;
					else if(counter_base == 8) counter_base = 4;
					else if(counter_base == 4) counter_base = 2;
				}//2번 스위치, 백의자리 숫자 + 1
				else if(n == 1 && shmaddr1[3] == 1){
					counter_number += counter_base*counter_base;
				}//3번 스위치, 십의자리 숫자 + 1
				else if(n == 1 && shmaddr1[4] == 1){
					counter_number += counter_base;
				}//4번 스위치, 일의자리 숫자 + 1
				else if(n == 1 && shmaddr1[5] == 1){
					counter_number += 1;
				}
				counter_number = counter_number % (counter_base*counter_base*counter_base);

				//바뀐 숫자를 FND에 출력. output process에 메세지 전달.
				shmaddr2[3] = '\0';
				shmaddr2[1] = counter_number % 100;
				shmaddr2[2] = counter_number / 100;
				shmaddr2[13] = counter_base;
				shmaddr2[0] = FND_WITH_BASE;
				sleep(1);
				
				//진수에 맞게 LED출력. output process에 메세지 전달.
				shmaddr2[2] = 0;
				if(counter_base == 2) {shmaddr2[1] = 28; shmaddr2[2] = 1;}
				else if(counter_base == 10) shmaddr2[1] = 64;
				else if(counter_base == 8) shmaddr2[1] = 32;
				else if(counter_base == 4) shmaddr2[1] = 16;
				shmaddr2[0] = LED;
				sleep(1);
			}
			else if(mode == TEXT_MODE){
				if(n == 1){ //동시에 눌려진 스위치가 한개이다.
					text_count = (text_count + 1)%10000;
					
					if(text_mode == 0){ //알파벳 입력 모드일 경우
						int change_all = 0;
						if(shmaddr1[2] == 1){// .QZ
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
						else if(shmaddr1[3] == 1){ //ABC
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
						else if(shmaddr1[4] == 1){ //DEF
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
						else if(shmaddr1[5] == 1){ //GHI
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
						else if(shmaddr1[6] == 1){ //JKL
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
						else if(shmaddr1[7] == 1){ //MNO
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
						else if(shmaddr1[8] == 1){ //PRS
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
						else if(shmaddr1[9] == 1){ //TUV
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
						else if(shmaddr1[10] == 1){ //WXY
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
						
						if(change_all == 1){ //한개씩 앞으로 미룬다.
							for(i=0; i<7; i++){
								text_buf[i] = text_buf[i+1];
							}
							text_buf[7] = tmp;
						}
						
					}
					else{//숫자 입력 모드일 경우
						for(i=0; i<9 ;i++){
							if(shmaddr1[2+i] == 1){
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
				else if(n == 2){//동시에 스위치 2개가 눌려진 경우
					text_count = (text_count + 1)%10000;
					
					if(shmaddr1[3] == 1 && shmaddr1[4] == 1){ //2,3번 버튼이 눌려짐
						for(i=0; i<8; i++) //초기화
							text_buf[i] = ' ';
						previous_char = ' ';
					}
					else if(shmaddr1[6] == 1 && shmaddr1[7] == 1){//5,6번 버튼이 눌려짐, alphabet <-> number
						if(text_mode == 0) text_mode = 1; //change to number mode
						else text_mode = 0; //change to alphabet mode
						previous_char = ' ';
						//DOT MATRIX 출력을 바꾼다. output process에 메세지 전달.
						shmaddr2[3] = '\0';
						shmaddr2[1] = text_mode;
						shmaddr2[2] = 0;
						shmaddr2[0] = DOT;
						sleep(1);
					}
					else if(shmaddr1[9] == 1 && shmaddr1[10] == 1){//8,9번 버튼이 눌려짐, insert a blank at the end
						for(i=0; i<7; i++){
							text_buf[i] = text_buf[i+1];
						}
						text_buf[7] = ' ';
						previous_char = ' ';
					}
				}
				//TEXT LCD 출력한다.
				//text_count를 FND 출력한다. -> output process에 메세지 전달.
				shmaddr2[3] = '\0';
				shmaddr2[1] = text_count % 100;
				shmaddr2[2] = text_count / 100;
				shmaddr2[0] = FND;
				sleep(1);
			}
			else if(mode == DRAW_MODE){
			if(n == 1){
				draw_count = (draw_count + 1)%10000;
				
				if(shmaddr1[2] == 1){ //1번 스위치, 그림을 reset
					for(i=0; i<10; i++){
						draw_board[i] = 0;
					}
					cursorX = 0; //커서 위치 reset
					cursorY = 6;
					isCursor = 1;
				}
				else if(shmaddr1[3] == 1){ //2번 스위치, 커서를 위로 이동
					if(cursorX > 0) cursorX--;
				}
				else if(shmaddr1[4] == 1){ //3번 스위치, 커서 보이기 <-> 커서 숨기기
					if(isCursor == 1) isCursor = 0;
					else isCursor = 1;
				}
				else if(shmaddr1[5] == 1){//4번 스위치, 커서 왼쪽으로 이동
					if(cursorY < 6) cursorY++;
				}
				else if(shmaddr1[6] == 1){//5번 스위치, 현재위치 선택
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
				else if(shmaddr1[7] == 1){//6번 스위치, 커서 오른쪽으로 이동
					if(cursorY > 0) cursorY--;
				}
				else if(shmaddr1[8] == 1){//7번 스위치, 그림 reset
					for(i=0; i<10; i++){
						draw_board[i] = 0;
					}
				}
				else if(shmaddr1[9] == 1){//8번 스위치, 커서 아래로 이동
					if(cursorX < 9) cursorX++;
				}
				else if(shmaddr1[10] == 1){//9번 스위치, 그림 반전시키기
					for(i=0; i<10; i++){
						draw_board[i] = ~draw_board[i] % 128;
					}
				}
				//그림 정보를 buf2.text에 담아 output process에 전달
				for(i=0; i<10; i++)
					shmaddr2[3+i] = draw_board[i];
				shmaddr2[1] = 2;
				shmaddr2[2] = 0;
				shmaddr2[0] = DOT;
				sleep(1);
				
				//draw_count정보를 FND에 출력
				shmaddr2[3] = '\0';
				shmaddr2[1] = draw_count%100;
				shmaddr2[2] = draw_count/100;
				shmaddr2[0] = FND;
				sleep(1);
			}
		}
		}
		sleep(1);
	}
}

/* event 버튼이 눌려진 경우 메세지를 받아서 처리하는 함수 */
void change_mode(){

	int shmid2 = shmget((key_t)1002, 14, IPC_CREAT|0666);
	char*shmaddr2 = (char*)shmat(shmid2, (char*)NULL, 0);
	int i;
	
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	//led 초기화
	shmaddr2[3] = '\0';
	shmaddr2[1] = 28;
	shmaddr2[2] = 1;
	shmaddr2[0] = LED;
	sleep(1);
	
	//text fnd 초기화
	hour = tm->tm_hour;
	minuit=tm->tm_min;
	shmaddr2[3] = '\0';
	shmaddr2[1] = minuit;
	shmaddr2[2] = 0;
	shmaddr2[0] = FND;
	sleep(1);
	
	int shmid3 = shmget((key_t)1003, 2, IPC_CREAT|0666);
	char* shmaddr = (char*)shmat(shmid3, (char*)NULL, 0);
	char whichButton;

	while(1){
		
		if( *shmaddr == EVENT ){
			*shmaddr = '*';
			printf("event msg received\n");
			whichButton = shmaddr[1];
			printf("%d\n",whichButton);			
			if(whichButton == 115){ //volume up
				mode = (mode + 1) % 4;
				printf("mode changed : %d\n", mode);
			}
			else if(whichButton == 114){ //volume down
				mode--;
				if(mode < 0) mode = mode + 4;
				printf("mode changed : %d\n", mode);
			}
			else if(whichButton == 116){ //PROG버튼, 초기화하고 프로세스를 종료한다.
				//DOT MATRIX 초기화
				shmaddr2[3] = '\0';
				shmaddr2[1] = -1;
				shmaddr2[2] = 0;
				shmaddr2[0] = DOT;
				sleep(1);
				
				//text fnd 초기화
				shmaddr2[3] = '\0';
				shmaddr2[1] = 0;
				shmaddr2[2] = 0;
				shmaddr2[0] = FND;
				sleep(1);
				
				//led 초기화
				shmaddr2[3] = '\0';
				shmaddr2[1] = 0;
				shmaddr2[2] = 0;
				shmaddr2[0] = LED;
				sleep(1);
				
				kill(pid_in, SIGINT);
				kill(pid_out, SIGINT);
				printf("Good bye\n");
				exit(0);
			}
		}
		
		/* initializaiton when mode changed */
		if(mode == CLOCK_MODE){
			isCursor = 0;
			flag = 0;
			//DOT MATRIX 빈칸으로 초기화
			/*buf2.type = DOT;
			buf2.num = -1;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//text fnd 현재 시간으로 초기화
			hour = tm->tm_hour;
			minuit=tm->tm_min;
			buf2.type = FND;
			buf2.num = hour*100 + minuit;
			strcpy(buf2.text, "        ");
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//led 1번에 불이 들어온다.
			buf2.type = LED;
			buf2.num = 128;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}*/
		}
		else if(mode == COUNTER_MODE){
			flag = 0;
			isCursor = 0;
			//DOT MATRIX 빈칸으로 초기화
			/*buf2.type = DOT;
			buf2.num = -1;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//text fnd 0으로 초기화
			buf2.type = FND;
			buf2.num = 0;
			strcpy(buf2.text, "        ");
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			counter_number = 0;
			counter_base = 10;
			//led 초기화 2번으로 초기화
			buf2.type = LED;
			buf2.num = 64;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}*/
		}
		else if(mode == TEXT_MODE){
			text_mode = 0;
			previous_char = ' ';
			text_count = 0;
			strcpy(text_buf, "        ");
			isCursor = 0;
			//DOT MATRIX 알파벳 A로 초기화
			/*buf2.type = DOT;
			buf2.num = 0;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//text fnd 0으로 초기화
			buf2.type = FND;
			buf2.num = 0;
			strcpy(buf2.text, "        ");
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			//led 0으로 초기화
			buf2.type = LED;
			buf2.num = 0;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}*/
		}
		else if(mode == DRAW_MODE){
			flag = 0;
			isCursor  = 1;
			cursorX = 0;
			cursorY = 6;
			draw_count = 0;
			for(i=0; i<10; i++){
				draw_board[i] = 0;
			}
			//DOT MATRIX 빈칸으로 초기화
			/*buf2.type = DOT;
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
			//led 0으로 초기화
			buf2.type = LED;
			buf2.num = 0;
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}*/
		}

	}
}

/* 커서가 보일 경우, CLOCK_MODE 수정 중일 경우 1초마다 출력이 바뀌도록 메세지를 전달하는 함수 */
void snd_msg(){
	
	int i;
	key_t key2;

	while(1){
		if(isCursor == 1){ //커서가 보인다.
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

			switch(tmpY){ //커서 위치가 보인다.
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
			for(i=0; i<10; i++){ //중간에 바뀔 수도 있어서 다시 체크
				tmp_board[i] = draw_board[i];
			}
			
			switch(tmpY){ //커서 위치가 안보인다.
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
		if(flag == 1){ //CLOCK_MODE에서 시간 수정모드일 경우 3번, 4번 LED를 번갈아 깜빡이도록 설정한다.
			struct msgbuf buf2;
			memset(buf2.text, 0, sizeof(buf2.text));
			key2 = msgget((key_t)1002, IPC_CREAT|0666);
			
			buf2.type = LED;
			buf2.num = 32; //3번 LED
			if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
				printf("key 2 msgsnd error\n");
				exit(0);
			}
			
			sleep(1);
			if(flag == 1){ //중간에 바뀌었는지 체크
				buf2.num = 16; //4번 LED
				if(msgsnd(key2, (void*)&buf2, sizeof(buf2)-sizeof(long), IPC_NOWAIT) == -1){
					printf("key 2 msgsnd error\n");
					exit(0);
				}
			}
		}
		sleep(1);
	}
}
/* input process --메세지전달--> main --메세지전달--> output process */
/* 메인에서 받은 메세지를 처리하고 출력을 위한 메세지 전달을 통괄한다. */
int main(int argc, char *argv[]){
	int r_value;
	mode = CLOCK_MODE;
	
	int shmid1, shmid2, shmid3;
	char* shmaddr;
	
	shmid1 = shmget((key_t)1001, 11, IPC_CREAT|0666);
	shmid2 = shmget((key_t)1002, 14, IPC_CREAT|0666);
	shmid3 = shmget((key_t)1003, 2, IPC_CREAT|0666);
	
	if(shmid1<0)
		printf("error\n");
	
	shmaddr = (char*)shmat(shmid1, (char*)NULL, 0);
	*shmaddr = '*';
	shmaddr = (char*)shmat(shmid2, (char*)NULL, 0);
	*shmaddr = '*';
	shmaddr = (char*)shmat(shmid3, (char*)NULL, 0);
	*shmaddr = '*';
	
	pid_in = fork();
	if(pid_in == 0){
		input_process();
	}else{
		pid_out = fork();
		if(pid_out == 0){
			output_process();
		}
		else{ //change_mode, receive_msg, snd_msg 함수가 동시에 작동하고 있다.
			r_value = pthread_create(&p_thread[0], NULL, change_mode, (void *)NULL);
			r_value = pthread_create(&p_thread[1], NULL, receive_msg, (void *)NULL);
			//r_value = pthread_create(&p_thread[2], NULL, snd_msg, (void *)NULL);
			pthread_join(p_thread[0], (void**)NULL);
			pthread_join(p_thread[1], (void**)NULL);
			//pthread_join(p_thread[2], (void**)NULL);
		}
	}
	return 0;
}
