#include "header.h"

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

void dot_out(int mode){
	
	int i;
	int dev;
	
	dev = open(DOT_DEVICE, O_WRONLY);
	if(dev < 0){
		printf("Device open error : %s\n", DOT_DEVICE);
		exit(1);
	}
	
	unsigned char fpga_a[10];
	unsigned char fpga_1[10];
	unsigned char fpga_blank[10];
	
	memset(fpga_a, 0, sizeof(fpga_a));
	memset(fpga_1, 0, sizeof(fpga_1));
	memset(fpga_blank, 0, sizeof(fpga_blank));
	
	fpga_a[0] = 0x1c; fpga_a[1] = 0x36; fpga_a[2] = 0x63; fpga_a[3] = 0x63;
	fpga_a[4] = 0x63; fpga_a[5] = 0x7f; fpga_a[6] = 0x7f; fpga_a[7] = 0x63;
	fpga_a[8] = 0x63; fpga_a[9] = 0x63;
	
	for(i=0; i<10; i++){
		fpga_blank[i] = 0x00;
		fpga_1[i] = 0x0c;
	}
	
	fpga_1[1] = 0x1c;
	fpga_1[2] = 0x1c;
	fpga_1[9] = 0x1e;
	
	if(mode == -1){
		write(dev, fpga_blank, sizeof(fpga_blank));
	}
	else if(mode == 0){
		write(dev, fpga_a, sizeof(fpga_a));
	}
	else if(mode == 1){
		write(dev, fpga_1, sizeof(fpga_a));
	}
	
	close(dev);
	return;
	
}

void dot_draw(int isCursor, int cursorX, int cursorY){
	
	int i;
	int dev;
	
	dev = open(DOT_DEVICE, O_WRONLY);
	if(dev < 0){
		printf("Device open error : %s\n", DOT_DEVICE);
		exit(1);
	}
	
	write(dev, draw_board, sizeof(draw_board));
	
	close(dev);
	return;
}

void text_out(const char* buf){
	unsigned char string[32];

	memset(string, ' ' , sizeof(string));
	strncpy(string, buf, 8);
	int dev;
	dev = open(TEXT_LCD_DEVICE, O_WRONLY);
	if(dev < 0){
		printf("Device open error : %s\n", TEXT_LCD_DEVICE);
		exit(1);
	}
	
	write(dev, string, sizeof(string));
	close(dev);
	return;
}

void cursor_blink(){

	while(1){
		while(isCursor == 1){
			int dev;
			
			dev = open(DOT_DEVICE, O_WRONLY);
			if(dev < 0){
				printf("Device open error : %s\n", DOT_DEVICE);
				exit(1);
			}
			
			switch(cursorY){
				case 7: draw_board[cursorX] = draw_board[cursorX] | 0b10000000; break;
				case 6: draw_board[cursorX] = draw_board[cursorX] | 0b01000000; break;
				case 5: draw_board[cursorX] = draw_board[cursorX] | 0b00100000; break;
				case 4: draw_board[cursorX] = draw_board[cursorX] | 0b00010000; break;
				case 3: draw_board[cursorX] = draw_board[cursorX] | 0b00001000; break;
				case 2: draw_board[cursorX] = draw_board[cursorX] | 0b00000100; break;
				case 1: draw_board[cursorX] = draw_board[cursorX] | 0b00000010; break;
				case 0: draw_board[cursorX] = draw_board[cursorX] | 0b00000001; break;
			}

			write(dev, draw_board, sizeof(draw_board));
			sleep(1);
			
			switch(cursorY){
				case 7: draw_board[cursorX] = draw_board[cursorX] & 0b01111111; break;
				case 6: draw_board[cursorX] = draw_board[cursorX] & 0b10111111; break;
				case 5: draw_board[cursorX] = draw_board[cursorX] & 0b11011111; break;
				case 4: draw_board[cursorX] = draw_board[cursorX] & 0b11101111; break;
				case 3: draw_board[cursorX] = draw_board[cursorX] & 0b11110111; break;
				case 2: draw_board[cursorX] = draw_board[cursorX] & 0b11111011; break;
				case 1: draw_board[cursorX] = draw_board[cursorX] & 0b11111101; break;
				case 0: draw_board[cursorX] = draw_board[cursorX] & 0b11111110; break;
			}
			
			write(dev, draw_board, sizeof(draw_board));
			
			close(dev);
			sleep(1);
		}
	}
}

void output_process(){
	
	key_t key1, key2, key3;
	struct msgbuf buf;
	key2 = msgget((key_t)1002, IPC_CREAT|0666);
	
	r_value = pthread_create(&p_thread[2], NULL, cursor_blink, NULL);
	pthread_join(p_thread[2], (void**)NULL);

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
			
			if(strcmp(buf.text, "") != 0){
				text_out(buf.text);
			}
		}
	}
	
}
