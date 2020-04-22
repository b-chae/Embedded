#include "header.h"

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
		|| push_sw_buff[4] == 1 || push_sw_buff[5] == 1 || push_sw_buff[6] == 1 || push_sw_buff[7] == 1 || push_sw_buff[8] == 1){
			read(dev, &push_sw_buff, sizeof(push_sw_buff));
			
			if(push_sw_buff[0] == 1 || push_sw_buff[1] == 1 || push_sw_buff[2] == 1 || push_sw_buff[3] == 1
		|| push_sw_buff[4] == 1 || push_sw_buff[5] == 1 || push_sw_buff[6] == 1 || push_sw_buff[7] == 1 || push_sw_buff[8] == 1){
				buf.n = 0;
				
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
			}
		}
		
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
