#include "header.h"

/* fnd_out function : num을 base진수로 fnd에 출력한다. */
void fnd_out(int num, int base){
	int dev;
	unsigned char data[4];
	unsigned char retval;

	memset(data, 0, sizeof(data));
	
	data[0] = num/(base*base*base); //천의 자리수
	data[1] = num/(base*base)%base; //백의 자리수
	data[2] = num%(base*base)/base; //십의 자리수
	data[3] = num%base; //일의 자리수

	dev = open(FND_DEVICE, O_RDWR);
	if(dev < 0){
		printf("Device open error %s\n", FND_DEVICE);
		exit(1);
	}
	retval = write(dev, &data, 4);
	
	memset(data, 0, sizeof(data));
	
	close(dev);
}

/* dot_out : dot matrix를 설정값에 따라 다르게 출력한다.
 * mode == -1 : 빈 화면 출력한다.
 * mode == 0 : 알파벳 A를 출력한다.
 * mode == 1 : 숫자 1을 출력한다.
 */
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
	fpga_1[1] = 0x1c; fpga_1[2] = 0x1c; fpga_1[9] = 0x1e;
	
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

/* dot_draw function : dot matrix를 draw_board 배열값에 맞추어 출력한다 */
void dot_draw(unsigned char* draw_board){	
	int dev;

	dev = open(DOT_DEVICE, O_WRONLY);
	if(dev < 0){
		printf("Device open error : %s\n", DOT_DEVICE);
		exit(1);
	}

	write(dev, draw_board, 10);
	close(dev);
	
	return;
}

/* text_out function : text lcd를 buf 배열값에 맞추어 출력한다. */
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

/* led_out function : mmap을 사용하여 n값을 led에 출력한다. */
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

/* output 출력 통괄, 메세지를 받으면 각각 맞는 출력을 할 수 있도록 */
void output_process(){
	
	int shmid2 = shmget((key_t)1002, 14, 0);
	char* shmaddr = (char*)shmat(shmid2, (char*)NULL, 0);
	char text[10];
	memset(text, 0, sizeof(text));
	int i;
	
	while(1){ //계속 메세지를 기다린다.
		if(*shmaddr != '*' && *shmaddr != '#'){
			int type = *shmaddr;
			*shmaddr = '#';
			printf("(%d %d)", shmaddr[1], shmaddr[2]);
			int n = shmaddr[1] + shmaddr[2]*100;
			for(i=0; i<10; i++){
				text[i] = shmaddr[3+i];
			}
			printf(" hoihoi ");
			printf("message received in output process [%d] %d [%s]\n", type,n,text);
			for(i=0; i<10; i++){
				printf("%d ",shmaddr[3+i]);
			}
			printf("\n");
			
			if(type == FND){ //FND 타입일 경우
				fnd_out(n, 10); //10진수를 기본으로 출력한다.
				if(strcmp(text, "") != 0){ //text출력할 것이 있으면 한다.
					text[8] = '\0';
					text_out(text);
				}
			}
			else if(type == FND_WITH_BASE){ //FND_WITH_BASE 타입일 경우
				fnd_out(n, shmaddr[13]); //buf.num을 buf.base진수로 하여 출력한다.
				if(strcmp(text, "")!=0){ //text출력할 것이 있으면 한다.
					text[8] = '\0';
					text_out(text);
				}
			}
			else if(type == DOT){ //DOT타입일 경우
				if(n == -1 || n == 0 || n == 1){ //특정 모드일 때는
					dot_out(n); //특정 모드에 맞는 dot matrix를 출력한다.
				}
				else{//그렇지 않을 경우에는 buf.text에 dot matrix 정보가 들어있다.
					dot_draw(text);
				}
			}
			else if(type == LED){//LED타입일 경우 해당 숫자를 출력한다.
				led_out(n);
			}
			*shmaddr = '*';
		}
		usleep(1000);
	}
}
