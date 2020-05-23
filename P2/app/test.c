/* FPGA FND Test Application
File : fpga_test_fnd.c
Auth : largest@huins.com */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>

#define MAX_DIGIT 4
#define FND_DEVICE "/dev/dev_driver"

struct mydata{
	int timer_interval;
	int timer_count;
	int timer_init;
};

struct mydata my_option;

#include "../module/ioctl.h"

int main(int argc, char **argv)
{
	int dev;
	unsigned char retval;
	int i;
	int str_size;
	int tmp;

	if(argc!=4) {
		printf("please input the parameter! \n");
		printf("ex)./app TIMER_INTERVAL[1-100] TIMER_CNT[1-100] TIMER_INIT[0001-8000]\n");
		return -1;
	}

	//TIMER_INTERVAL
	tmp = atoi(argv[1]);
	if(tmp < 0 || tmp > 100){
		printf("TIMER_INTERVAL out of range");
		exit(1);
	}
	my_option.timer_interval = tmp;

	//TIMER_CNT
	tmp = atoi(argv[2]);
	if(tmp < 0 || tmp > 100){
		printf("TIMER_CNT out of range");
		exit(1);
	}
	my_option.timer_count = tmp;

	//TIMER_INIT parameter 처리
    str_size=(strlen(argv[3]));
    if(str_size>MAX_DIGIT)
    {
        printf("Warning! TIMER_INIT 4 Digit number only!\n");
        str_size=MAX_DIGIT;
		exit(1);
    }

    for(i=0;i<str_size;i++)
    {
        if((argv[3][i]<0x30)||(argv[3][i])>0x39) {
            printf("Error! Invalid Value!\n");
            return -1;
        }
    }
	my_option.timer_init = atoi(argv[3]);

    dev = open(FND_DEVICE, O_RDWR);
    if (dev<0) {
        printf("Device open error : %s\n",FND_DEVICE);
        exit(1);
    }

    //retval=write(dev,&my_option,sizeof(my_option));	
	/* IOCTL을 사용하여 사용자 옵션 전달*/
	retval = ioctl(dev, IOCTL_SEND_ARG, &my_option);
    if(retval<0) {
        printf("ioctl1 Error!\n");
        return -1;
    }
	
	/* IOCTL을 사용하여 프로그램 시작*/
	retval = ioctl(dev, IOCTL_START, NULL);
    if(retval<0) {
        printf("ioctl2 Error!\n");
        return -1;
    }

	close(dev);

	return(0);
}
