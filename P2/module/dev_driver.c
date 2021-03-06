#include "fpga_dot.h"
#include "driver_header.h"
#include "ioctl.h"
#include "write.h"

int iom_device_release(struct inode *minode, struct file *mfile) {
	printk("dev_driver_release\n");
	device_usage = 0;
	return 0;
}

int iom_device_open(struct inode *minode, struct file *mfile) {
	printk("dev_driver_open\n");
	if (device_usage != 0) {
		return -EBUSY;
	}
	device_usage = 1;
	return 0;
}

/* ioctl 명령 실행 */
static long iom_device_ioctl(struct file *mfile, unsigned int cmd, unsigned long arg){
	int ret;
	struct timer_option param;
	
	switch(cmd){
		case IOCTL_SEND_ARG: //option전달
			ret = copy_from_user((void*)&param, (void*)arg, sizeof(param));
			if(ret < 0){
				printk("copy_from_user error\n");
				return -1;
			}
			
			//사용자가 전해 준 옵션(timer_interval, timer_count, timer_init)을 저장한다. 
			option.timer_interval = param.timer_interval;
			option.timer_count = param.timer_count;
			option.timer_init = param.timer_init;
			printk("IOCTL_SEND_ARG COMPLETE\n");		
			break;
		case IOCTL_START: //device 구동
			deal_with_data();

			del_timer_sync(&mydata.timer);

			mydata.timer.expires = jiffies + (option.timer_interval * HZ / 10);
			mydata.timer.data = (unsigned long)&mydata;
			mydata.timer.function = timer_func;
			
			add_timer(&mydata.timer);
			printk("TIMER START\n");
			break;
	}
	return 0;
}

/* 사용자 옵션 정보를 정제하여 mydata에 넣기 */
void deal_with_data(void){
	unsigned char value[4];
	unsigned short real_value = 0;
	
	/* 현재 option.timer_init은 4-digit integer로 되어있다. 이것을 자리수별로 끊기 */
	value[0] = option.timer_init/1000;
	value[1] = option.timer_init/100%10;
	value[2] = option.timer_init%100/10;
	value[3] = option.timer_init%10;
	
	/* 0이 아닌 숫자의 값과 인덱스 real_value와 fnd_index 변수에 넣기 */
	if(value[0] != 0){
		real_value = value[0];
		mydata.fnd_index = 0;
	}
	else if(value[1] != 0){
		real_value = value[1];
		mydata.fnd_index = 1;
	}
	else if(value[2] != 0){
		real_value = value[2];
		mydata.fnd_index = 2;
	}
	else if(value[3] != 0){
		real_value = value[3];
		mydata.fnd_index = 3;
	}
	
	/* 초기화 */
	mydata.count = 0;
	mydata.rotation_count = 0;
	mydata.current_num = real_value;
	mydata.text_index_i = 0;
	mydata.text_index_j = 0;
	mydata.i_direction = 1;
	mydata.j_direction = 1;
	
	/* 출력 초기화 */
	fnd_write(real_value, mydata.fnd_index);
	dot_write(real_value);
	led_write(real_value);
	text_write(0,0);
}

/* timer_func가 수행되기 전 데이터 업데이트 */
void update_data(void){
	
	/* 8번마다 fnd출력되는 index 위치를 하나 오른쪽으로 옮긴다 */
	mydata.rotation_count++;
	if(mydata.rotation_count >= 8){
		mydata.rotation_count = 0;
		
		mydata.fnd_index++;
		/* 오른쪽 끝에서는 -> 맨 왼쪽으로 옮긴다 */
		if(mydata.fnd_index >= 4){
			mydata.fnd_index = 0;
		}
	}
	
	/* 현재 출력되는 숫자가 1->2->3->...->8->1->2-> ... 로 계속 바뀐다 */
	mydata.current_num++;
	if(mydata.current_num >= 9){
		mydata.current_num = 1;
	}
	
	/* 학번 출력이 좌우로 왔다갔다 할 수 있도록 방향과 인덱스를 바꾼다 */
	if(mydata.i_direction == 1){//오른쪽으로 움직인다.
		if(mydata.text_index_i + STRLEN_STUDENT_NUMBER - 1 < 15){
			mydata.text_index_i++;
		}
		else{//오른쪽 끝인 경우 방향을 바꾼다.
			mydata.i_direction = -1;
			mydata.text_index_i--;
		}
	}
	else{//왼쪽으로 움직인다.
		if(mydata.text_index_i > 0){
			mydata.text_index_i--;
		}
		else{//왼쪽 끝인 경우 방향을 바꾼다.
			mydata.i_direction = 1;
			mydata.text_index_i++;
		}
	}
	
	/* 이름 출력이 좌우로 왔다갔다 할 수 있도록 방향과 인덱스를 바꾼다 */
	if(mydata.j_direction == 1){//오른쪽으로 움직인다.
		if(mydata.text_index_j + STRLEN_MY_NAME - 1 < 15){
			mydata.text_index_j++;
		}
		else{//오른쪽 끝인 경우 방향을 바꾼다.
			mydata.j_direction = -1;
			mydata.text_index_j--;
		}
	}
	else{//왼쪽으로 움직인다.
		if(mydata.text_index_j > 0){
			mydata.text_index_j--;
		}
		else{//왼쪽끝인 경우 방향을 바꾼다.
			mydata.j_direction = 1;
			mydata.text_index_j++;
		}
	}
}

/* timer가 timer_interval마다 구동하는 함수 */
static void timer_func(unsigned long timeout) {
	struct struct_mydata *p_data = (struct struct_mydata*)timeout;
	printk("timer_func %d\n", p_data->count);

	p_data->count++;
	if( p_data->count >= option.timer_count ) {
		clear_device(); //timer_count번 수행하면 초기화하고 종료
		return;
	}

	mydata.timer.expires = get_jiffies_64() + (option.timer_interval * HZ / 10);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = timer_func;

	add_timer(&mydata.timer);
	
	update_data();
	
	fnd_write(mydata.current_num, mydata.fnd_index);
	dot_write(mydata.current_num);
	led_write(mydata.current_num);
	text_write(mydata.text_index_i,mydata.text_index_j);
}

ssize_t iom_device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {

	printk("not implemented yet\n");
	return 1;
}

int __init iom_device_init(void)
{	
	printk("dev_driver_init\n");

	major = register_chrdev(IOM_DEVICE_MAJOR, IOM_DEVICE_NAME, &iom_device_fops);
	if(major <0) {
		printk( "error %d\n",major);
		return major;
	}
	printk( "dev_file : /dev/%s , major : %d\n",IOM_DEVICE_NAME, IOM_DEVICE_MAJOR);

	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
	iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
	iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);

	init_timer(&(mydata.timer));

	printk("init module\n");
	return 0;
}

void __exit iom_device_exit(void)
{
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_dot_addr);
	iounmap(iom_fpga_text_lcd_addr);
	
	printk("dev_driver_exit\n");
	device_usage = 0;
	del_timer_sync(&mydata.timer);

	unregister_chrdev(IOM_DEVICE_MAJOR, IOM_DEVICE_NAME);
}

module_init(iom_device_init);
module_exit(iom_device_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
