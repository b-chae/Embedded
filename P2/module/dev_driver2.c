#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/version.h>

#define IOM_DEVICE_MAJOR 242		// ioboard fpga device major number
#define IOM_DEVICE_NAME "dev_driver"		// ioboard fpga device name

#define IOM_FND_ADDRESS 0x08000004 // pysical address
#define IOM_LED_ADDRESS 0x08000016 // pysical address
#define IOM_FPGA_DOT_ADDRESS 0x08000210
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090

#define STRLEN_STUDENT_NUMBER 8
#define STRLEN_MY_NAME 11

#define KERNEL_TIMER_NAME "dev_driver2"

//Global variable
static int device_usage = 0;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;

const char* student_number = "20171696";
const char* my_name = "byeori chae";

unsigned char fpga_number[10][10] = {
	{0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03} // 9
};

unsigned char fpga_set_full[10] = {
	// memset(array,0x7e,sizeof(array));
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f
};

unsigned char fpga_set_blank[10] = {
	// memset(array,0x00,sizeof(array));
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// define functions...
ssize_t iom_device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_device_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_device_open(struct inode *minode, struct file *mfile);
int iom_device_release(struct inode *minode, struct file *mfile);
void fnd_write(const char* value);
void dot_write(int n);
void led_write(unsigned char n);
void text_write(int, int);

// define file_operations structure 
static struct file_operations iom_device_fops =
{
	.owner		=	THIS_MODULE,
	.open		=	iom_device_open,
	.write		=	iom_device_write,	
	.read		=	iom_device_read,	
	.release	=	iom_device_release
};

static struct mydata{
	int timer_interval;
	int timer_count;
	int timer_init;
};

static struct struct_timer_data{
	struct timer_list timer;
	int count;
};

struct mydata option;
struct struct_timer_data mydata;
int major;

// when fnd device close ,call this function
int iom_device_release(struct inode *minode, struct file *mfile) 
{
	device_usage = 0;

	return 0;
}

// when fnd device open ,call this function
int iom_device_open(struct inode *minode, struct file *mfile) 
{	
	if(device_usage != 0) return -EBUSY;
	
	device_usage = 1;

	return 0;
}

static void kernel_timer_blink(unsigned long timeout) {
	struct struct_mydata *p_data = (struct struct_mydata*)timeout;
	printk("kernel_timer_blink %d\n", p_data->count);

	p_data->count++;
	if( p_data->count >= option.timer_count ) {
		return;
	}

	mydata.timer.expires = get_jiffies_64() + (option.timer_interval * 0.1 * HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_blink;

	add_timer(&mydata.timer);
}

ssize_t iom_device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	const char *tmp = gdata;
	char kernel_timer_buff = 0;
	unsigned char value[4];
	unsigned char real_value = 0;

	printk("write\n");
	// 1 byte
	if (copy_from_user(&option, gdata, sizeof(option))) {
		return -EFAULT;
	}
	
	value[0] = option.timer_init/1000;
	value[1] = option.timer_init/100%10;
	value[2] = option.timer_init%100/10;
	value[3] = option.timer_init%10;
	
	fnd_write(value);
	
	if(value[0] != 0){
		real_value = value[0];
	}
	else if(value[1] != 0){
		real_value = value[1];
	}
	else if(value[2] != 0){
		real_value = value[2];
	}
	else if(value[3] != 0){
		real_value = value[3];
	
	dot_write(real_value);
	led_write(real_value);
	text_write(0, 0);

	mydata.count = 0;

	printk("data  : %d \n",mydata.count);

	del_timer_sync(&mydata.timer);

	mydata.timer.expires = jiffies + (0.1 * option.timer_interval * HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_blink;

	add_timer(&mydata.timer);
	return 1;
}

void fnd_write(const char* value){
	unsigned short int value_short;
	value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);
}

void dot_write(int n){
	int i;
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

	init_timer(&(mydata.timer));

	printk("init module\n");
	return 0;
}

void __exit iom_device_exit(void)
{
	printk("kernel_timer_exit\n");
	kernel_timer_usage = 0;
	del_timer_sync(&mydata.timer);

	unregister_chrdev(major, IOM_DEVICE_NAME);
}

module_init(iom_device_init);
module_exit( iom_device_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
