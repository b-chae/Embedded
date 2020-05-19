#include "driver_header.h"

int iom_device_release(struct inode *minode, struct file *mfile) {
	printk("kernel_timer_release\n");
	device_usage = 0;
	return 0;
}

int iom_device_open(struct inode *minode, struct file *mfile) {
	printk("kernel_timer_open\n");
	if (device_usage != 0) {
		return -EBUSY;
	}
	device_usage = 1;
	return 0;
}

void update_data(void){
	
	mydata.rotation_count++;
	if(mydata.rotation_count >= 8){
		mydata.rotation_count = 0;
		
		mydata.fnd_index++;
		if(mydata.fnd_index >= 4){
			mydata.fnd_index = 0;
		}
	}
	mydata.current_num++;
	if(mydata.current_num >= 9){
		mydata.current_num = 1;
	}
	
	if(mydata.i_direction == 1){
		if(mydata.text_index_i + STRLEN_STUDENT_NUMBER - 1 < 16){
			mydata.text_index_i++;
		}
		else{
			mydata.i_direction = -1;
			mydata.text_index_i--;
		}
	}
	else{
		if(mydata.text_index_i > 0){
			mydata.text_index_i--;
		}
		else{
			mydata.i_direction = 1;
			mydata.text_index_i++;
		}
	}
	
	if(mydata.j_direction == 1){
		if(mydata.text_index_j + STRLEN_MY_NAME - 1 < 32){
			mydata.text_index_j++;
		}
		else{
			mydata.j_direction = -1;
			mydata.text_index_j--;
		}
	}
	else{
		if(mydata.text_index_j > 16){
			mydata.text_index_j--;
		}
		else{
			mydata.j_direction = 1;
			mydata.text_index_j++;
		}
	}
}

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

static void kernel_timer_blink(unsigned long timeout) {
	struct struct_mydata *p_data = (struct struct_mydata*)timeout;
	printk("kernel_timer_blink %d\n", p_data->count);

	p_data->count++;
	if( p_data->count >= option.timer_count ) {
		clear_device();
		return;
	}

	mydata.timer.expires = get_jiffies_64() + (option.timer_interval * HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_blink;

	add_timer(&mydata.timer);
	
	update_data();
	
	fnd_write(mydata.current_num, mydata.fnd_index);
	dot_write(mydata.current_num);
	led_write(mydata.current_num);
	text_write(mydata.text_index_i,mydata.text_index_j);
}

void deal_with_data(void){
	unsigned char value[4];
	unsigned short real_value = 0;
	
	value[0] = option.timer_init/1000;
	value[1] = option.timer_init/100%10;
	value[2] = option.timer_init%100/10;
	value[3] = option.timer_init%10;
	
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
	
	mydata.count = 0;
	mydata.rotation_count = 0;
	mydata.current_num = real_value;
	mydata.text_index_i = 0;
	mydata.text_index_j = 0;
	mydata.i_direction = 1;
	mydata.j_direction = 1;
	
	fnd_write(real_value, mydata.fnd_index);
	dot_write(real_value);
	led_write(real_value);
	text_write(0,0);
}

ssize_t iom_device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	// 1 byte
	if (copy_from_user(&option, gdata, sizeof(option))) {
		return -EFAULT;
	}
	
	deal_with_data();

	printk("data  : %d \n",mydata.count);

	del_timer_sync(&mydata.timer);

	mydata.timer.expires = jiffies + (option.timer_interval * HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_blink;
	
	add_timer(&mydata.timer);
	return 1;
}

void fnd_write(int n, int index){
	const char value[4];
	int i;
	for(i=0; i<4; i++) value[i] = 0;
	value[index] = n;
	unsigned short int value_short;
	value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);
}

void dot_write(int n){
	int i;
	if(n == -1){
		for(i=0; i<10; i++){
			outw(fpga_set_blank[i], (unsigned int)iom_fpga_dot_addr + 2*i);
		}
		return;
	}
	for(i=0; i<10; i++){
		outw(fpga_number[n][i], (unsigned int)iom_fpga_dot_addr + 2*i);
	}
}

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

void text_write(int l_index, int r_index) 
{
	int i;

	unsigned char value[32];
   	unsigned short int _s_value = 0;
	
	for(i=0; i<l_index; i++){
		value[i] = ' ';
	}
	for(i=l_index; i<l_index+STRLEN_STUDENT_NUMBER; i++){
		value[i] = student_number[i-l_index];
	}
	for(i=i; i<16; i++){
		value[i] = ' ';
	}
	
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
	
	printk("kernel_timer_init\n");

	major = register_chrdev(0, IOM_DEVICE_NAME, &iom_device_fops);
	if(major <0) {
		printk( "error %d\n",major);
		return major;
	}
	printk( "dev_file : /dev/%s , major : %d\n",IOM_DEVICE_NAME,major);

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
	
	printk("kernel_timer_exit\n");
	device_usage = 0;
	del_timer_sync(&mydata.timer);

	unregister_chrdev(major, IOM_DEVICE_NAME);
}

module_init(iom_device_init);
module_exit(iom_device_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
