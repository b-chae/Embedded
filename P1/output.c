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
	
	unsigned char fpga_a = {0x1c, 0x36, 0x63, 0x63, 0x63, 0x7f, 0x7f, 0x63, 0x63, 0x63};
	unsigned char fpga_1 = {0x0c, 0x1c, 0x1c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x1e};
	unsigned char fpga_blank = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
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

void text_out(){
	unsigned char string[32];
	memset(string, '1' , sizeof(string));
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
