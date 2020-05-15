/* FPGA FND Ioremap Control
FILE : fpga_fpga_driver.c 
AUTH : largest@huins.com */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>


#define IOM_DEVICE_MAJOR 242		// ioboard fpga device major number
#define IOM_DEVICE_NAME "dev_driver"		// ioboard fpga device name

#define IOM_FND_ADDRESS 0x08000004 // pysical address
#define IOM_LED_ADDRESS 0x08000016 // pysical address

//Global variable
static int fpga_fnd_port_usage = 0;
static unsigned char *iom_fpga_fnd_addr;
static int ledport_usage = 0;
static unsigned char *iom_fpga_led_addr;

// define functions...
ssize_t iom_device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_device_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_device_open(struct inode *minode, struct file *mfile);
int iom_device_release(struct inode *minode, struct file *mfile);

// define file_operations structure 
struct file_operations iom_device_fops =
{
	.owner		=	THIS_MODULE,
	.open		=	iom_device_open,
	.write		=	iom_device_write,	
	.read		=	iom_device_read,	
	.release	=	iom_device_write,
};

// when fnd device open ,call this function
int iom_device_open(struct inode *minode, struct file *mfile) 
{	
	if(fpga_fnd_port_usage != 0) return -EBUSY;
	if(ledport_usage != 0) return -EBUSY;

	fpga_fnd_port_usage = 1;
	ledport_usage = 1;

	return 0;
}

// when fnd device close ,call this function
int iom_device_release(struct inode *minode, struct file *mfile) 
{
	fpga_fnd_port_usage = 0;
	ledport_usage = 0;

	return 0;
}

// when write to fnd device  ,call this function
ssize_t iom_device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	int i;
	unsigned char value[4];
	unsigned short int value_short = 0;
	unsigned short _s_value = 0;
	const char *tmp = gdata;

	if (copy_from_user(&value, tmp, 4))
		return -EFAULT;

    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    if(value[0] != 0){
		if(value[0] == 1){
			_s_value = 128;
		}
		else if(value[0] == 2){
			_s_value = 64;
		}
		else if(value[0] == 3){
			_s_value = 32;
		}
		else if(value[0] == 4){
			_s_value = 16;
		}
		else if(value[0] == 5){
			_s_value = 8;
		}
		else if(value[0] == 6){
			_s_value = 4;
		}
		else if(value[0] == 7){
			_s_value = 2;
		}
		else if(value[0] == 8){
			_s_value = 1;
		}
	}
	
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);
	outw(_s_value, (unsigned int)iom_fpga_led_addr);

	return length;
}

// when read to fnd device  ,call this function
ssize_t iom_device_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) 
{
	int i;
	unsigned char value[4];
	unsigned short int value_short = 0;
	char *tmp = gdata;

    value_short = inw((unsigned int)iom_fpga_fnd_addr);	    
    value[0] =(value_short >> 12) & 0xF;
    value[1] =(value_short >> 8) & 0xF;
    value[2] =(value_short >> 4) & 0xF;
    value[3] = value_short & 0xF;

    if (copy_to_user(tmp, value, 4))
        return -EFAULT;

	return length;
}

int __init iom_device_init(void)
{
	int result;

	result = register_chrdev(IOM_DEVICE_MAJOR, IOM_DEVICE_NAME, &iom_fpga_fnd_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);

	printk("init module, %s major number : %d\n", IOM_DEVICE_NAME, IOM_DEVICE_MAJOR);

	return 0;
}

void __exit iom_device_exit(void) 
{
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_led_addr);
	unregister_chrdev(IOM_DEVICE_MAJOR, IOM_DEVICE_NAME);
}

module_init(iom_device_init);
module_exit(iom_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");