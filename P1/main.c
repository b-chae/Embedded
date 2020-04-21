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
