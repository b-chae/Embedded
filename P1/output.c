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