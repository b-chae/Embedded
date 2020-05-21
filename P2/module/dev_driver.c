#include "fpga_dot.h"
#include "driver_header.h"
#include "ioctl.h"

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
	struct mydata param;
	
	switch(cmd){
		case IOCTL_SEND_ARG: //option전달
			ret = copy_from_user((void*)&param, (void*)arg, sizeof(param));
			if(ret < 0){
				printk("copy_from_user error\n");
				return -1;
			}
			
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

/* 출력 초기화 */
void clear_device(void){
	int i;
	unsigned short _s_value;
	fnd_write(0, 0);
	dot_write(-1);
	led_write(0);
	for(i=0;i<32;i++)
    {
        _s_value = (' ' & 0xFF) << 8 | ' ' & 0xFF;
		outw(_s_value,(unsigned int)iom_fpga_text_lcd_addr+i);
        i++;
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

/* 사용자 옵션 정보를 정제하여 mydata에 넣기 */
void deal_with_data(void){
	unsigned char value[4];
	unsigned short real_value = 0;
	
	/* 현재 option.timer_init은 4-digit integer로 되어있다. 이것을 자리수별로 끊기 */
	value[0] = option.timer_init/1000;
	value[1] = option.timer_init/100%10;
	value[2] = option.timer_init%100/10;
	value[3] = option.timer_init%10;
	
	/* 0이 아닌 숫자의 값고 인덱스 real_value와 fnd_index 변수에 넣기 */
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

ssize_t iom_device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {

	printk("not implemented yet\n");
	return 1;
}

/* fnd 출력 
 * n이라는 1 digit 숫자를 index 번째에 출력
 * ex) n = 5, index = 3 일경우 0005
 *     n = 1, index = 1 일경우 0100
 */
void fnd_write(int n, int index){
	char value[4];
	int i;
	unsigned short int value_short;
	
	for(i=0; i<4; i++) value[i] = 0;
	value[index] = n;
	
	value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);
}

/* dot matrix 출력 */
void dot_write(int n){
	int i;
	if(n == -1){ //빈 matrix로 초기화
		for(i=0; i<10; i++){
			outw(fpga_set_blank[i], (unsigned int)iom_fpga_dot_addr + 2*i);
		}
		return;
	}
	for(i=0; i<10; i++){
		outw(fpga_number[n][i], (unsigned int)iom_fpga_dot_addr + 2*i);
	}
}

/* led 불켜기 */
void led_write(unsigned char n){
	
	unsigned short _s_value = 0;
	
	switch(n){
		case 1 : _s_value = 128; break;
		case 2 : _s_value = 64; break;
		case 3 : _s_value = 32; break;
		case 4 : _s_value = 16; break;
		case 5 : _s_value = 8; break;
		case 6 : _s_value = 4; break;
		case 7 : _s_value = 2; break;
		case 8 : _s_value = 1; break;
	}

	outw(_s_value, (unsigned int)iom_fpga_led_addr);
}

/* text lcd에 학번과 이름을 표시한다
 * l_index : 학번이 시작되는 index
 * r_index : 이름이 시작되는 index
 */
void text_write(int l_index, int r_index) 
{
	int i;

	unsigned char value[32];
   	unsigned short int _s_value = 0;
	
	/* l_index 위치부터 학번 print */
	for(i=0; i<l_index; i++){
		value[i] = ' ';
	}
	for(i=l_index; i<l_index+STRLEN_STUDENT_NUMBER; i++){
		value[i] = student_number[i-l_index];
	}
	for(i=i; i<16; i++){
		value[i] = ' ';
	}
	
	/* r_index 위치부터 이름 print */
	for(i=16; i<r_index+16; i++){
		value[i] = ' ';
	}
	for(i=i; i<r_index+16+STRLEN_MY_NAME; i++){
		value[i] = my_name[i-r_index-16];
	}
	for(i=i; i<32; i++){
		value[i] = ' ';
	}

	for(i=0;i<32;i++)
    {
        _s_value = (value[i] & 0xFF) << 8 | value[i + 1] & 0xFF;
	outw(_s_value,(unsigned int)iom_fpga_text_lcd_addr+i);
        i++;
    }
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
