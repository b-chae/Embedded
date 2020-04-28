#include "header.h"

/* 이벤트 입력을 받으면 main process에 입력 정보를 전달해준다. */
void event_input(){
	
	struct input_event ev[BUFF_SIZE];
	int fd, rd, value, size = sizeof(struct input_event);

	if((fd = open(EVENT_DEVICE, O_RDONLY)) == -1){
		printf("%s is not a valid device.\n", EVENT_DEVICE);
	}
	
	while(1){
		
		if((rd = read(fd, ev, size*BUFF_SIZE)) < size){
			printf("read()");
			return;
		}
		
		value = ev[0].value;
		
		if(value == KEY_PRESS){ //버튼이 눌렸다.
		
			int shmid3 = shmget((key_t)1003, 2, 0);
			char* shmaddr = (char*)shmat(shmid3, (char*)NULL, 0);
			
			shmaddr[1] = ev[0].code;
			*shmaddr = EVENT;
			printf("event button pressed\n");
			while(*shmaddr != '*') usleep(100);
			//메인 프로세스에 메세지 전달.
		}
		
	}
	
}

/* 스위치 입력을 받으면 main process에 입력 정보를 전달해준다. */
void switch_input(){
	
	int shmid1 = shmget((key_t)1001, 11, 0);
	char* shmaddr = (char*)shmat(shmid1, (char*)NULL, 0);
	
	int i;
	int dev;
	int flag;
	
	dev = open(SWITCH_DEVICE, O_RDWR);
	unsigned char push_sw_buff[9];
	memset(push_sw_buff, 0, sizeof(push_sw_buff));	

	if(dev < 0){
		printf("Device open error\n");
		close(dev);
		return;
	}
	
	while(1){
		shmaddr[1] = 0;
		flag = 0;
		
		while(push_sw_buff[0] == 0 && push_sw_buff[1] == 0 && push_sw_buff[2] == 0 && push_sw_buff[3] == 0 
		&& push_sw_buff[4] == 0 && push_sw_buff[5] == 0 && push_sw_buff[6] == 0 && push_sw_buff[7] == 0 
		&& push_sw_buff[8] == 0)
			read(dev, &push_sw_buff, sizeof(push_sw_buff));

		for(i=0; i<9; i++){
			if(push_sw_buff[i] == 1)
			{
				shmaddr[1] += 1; //몇 개의 버튼이 눌려졌는 지 센다.
				shmaddr[2+i] = 1; //눌러짐 1
			}
			else{
				shmaddr[2+i] = 0; //안눌려짐 2
			}
		}
		
		//버튼을 눌렀다가 뗄 때까지 기다린다.
		while(push_sw_buff[0] == 1 || push_sw_buff[1] == 1 || push_sw_buff[2] == 1 || push_sw_buff[3] == 1
		|| push_sw_buff[4] == 1 || push_sw_buff[5] == 1 || push_sw_buff[6] == 1 || push_sw_buff[7] == 1 || push_sw_buff[8] == 1){
			read(dev, &push_sw_buff, sizeof(push_sw_buff));
			
			/* 두개 입력받는 경우 처리 */
			if(flag == 0 && (push_sw_buff[0] == 1 || push_sw_buff[1] == 1 || push_sw_buff[2] == 1 || push_sw_buff[3] == 1
		|| push_sw_buff[4] == 1 || push_sw_buff[5] == 1 || push_sw_buff[6] == 1 || push_sw_buff[7] == 1 || push_sw_buff[8] == 1)){
				
				shmaddr[1] = 0;
				
				for(i=0; i<9; i++){
					if(push_sw_buff[i] == 1)
					{
						shmaddr[1] += 1;
						shmaddr[2+i] = 1;
					}
					else{
						shmaddr[2+i] = 0;
					}
				}
				
				if(shmaddr[1] == 2) flag = 1;
			}
		}
		
		printf("send switch message %d switches pressed\n", shmaddr[1]);
		for(i=0; i<9; i++){
			printf("%d ", shmaddr[2+i]);
		}
		printf("\n");
		*shmaddr = SWITCH;
		
		while(*shmaddr != '*') usleep(100);
	}
	close(dev);
}

/* input process를 총괄한다. */
void input_process(){
	
	/* 동시에 switch_input 함수와 event_input 함수가 돌아가고 있다. */
	int r_value;
	r_value = pthread_create(&p_thread[3], NULL, switch_input, NULL);
	r_value = pthread_create(&p_thread[4], NULL, event_input, NULL);
	pthread_join(p_thread[3], (void**)NULL);
	pthread_join(p_thread[4], (void**)NULL);
}
