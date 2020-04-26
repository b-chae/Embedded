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
	
	int dev;
	int i;
	
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

void dot_draw(unsigned char* draw_board){	
	int dev;

	dev = open(DOT_DEVICE, O_WRONLY);
	if(dev < 0){
		printf("Device open error : %s\n", DOT_DEVICE);
		exit(1);
	}
	
	int i;
	for(i=0 ;i<10;i++)
		printf("%d ", draw_board[i]);
	printf("\n");

	write(dev, draw_board, 10);
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

void led_out(char n){
	int fd, i;
	
	unsigned long *fpga_addr = 0;
	unsigned char *led_addr = 0;
	
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd < 0){
		printf("/dev/mem open error\n");
		exit(1);
	}
	
	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED)
	{
		printf("mmap error!\n");
		close(fd);
		exit(1);
	}
	
	led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);
	*led_addr = n;
	
	munmap(led_addr, 4096);
	close(fd);
}

void output_process(){
	
	int i;
	key_t key1, key2, key3;
	
	while(1){
		struct msgbuf buf;
		key2 = msgget((key_t)1002, IPC_CREAT|0666);
		
		if(msgrcv(key2, (void*)&buf, sizeof(buf) - sizeof(long), 0, MSG_NOERROR) == -1){
			printf("msgrcv error\n");
			exit(0);
		}
		else{
			printf("message received %d %d ", buf.type, buf.num);
			for(i=0; i<10; i++)
				printf("%d ", buf.text[i]);
			printf("\n");
			if(buf.type == FND){
				if(buf.type == 10){
						fnd_out(buf.num, 10);
				}
				if(strcmp(buf.text, "")!=0){
					buf.text[8] = '\0';
					text_out(buf.text);
				}
			}
			else if(buf.type == FND_WITH_BASE){
				fnd_out(buf.num, buf.base);
				if(strcmp(buf.text, "")!=0){
					buf.text[8] = '\0';
					text_out(buf.text);
				}
			}
			else if(buf.type == DOT){
				if(buf.num == -1 || buf.num == 0 || buf.num == 1){
					dot_out(buf.num);
				}
				else{
					dot_draw(buf.text);
				}
			}
			else if(buf.type == LED){
				led_out(buf.num);
			}
		}
	}
}
