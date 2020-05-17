#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define IOM_DEVICE_MAJOR 242
#define IOM_DEVICE_NAME "dev_driver2"

static int device_usage = 0;

int iom_device_open(struct inode *, struct file *);
int iom_device_release(struct inode *, struct file *);
ssize_t iom_device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations iom_device_fops =
{ .open = iom_device_open, .write = iom_device_write,
	.release = iom_device_release };

static struct struct_mydata {
	struct timer_list timer;
	int count;
};

struct mydata{
	int timer_interval;
	int timer_count;
	int timer_init;
};

struct struct_mydata mydata;
struct mydata option;
int major;
char* howmany;

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
	howmany = kmalloc(PAGE_SIZE, GFP_KERNEL);
	return 0;
}

static void kernel_timer_blink(unsigned long timeout) {
	struct struct_mydata *p_data = (struct struct_mydata*)timeout;
	howmany[0]++;
	printk("kernel_timer_blink %d\n", p_data->count);

	p_data->count++;
	if( p_data->count >= option.timer_count ) {
		return;
	}

	mydata.timer.expires = get_jiffies_64() + (3 * HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_blink;

	add_timer(&mydata.timer);
}

ssize_t iom_device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	const char *tmp = gdata;
	char kernel_timer_buff = 0;

	printk("write\n");
	// 1 byte
	if (copy_from_user(&option, gdata, sizeof(option))) {
		return -EFAULT;
	}

	mydata.count = 0;

	printk("data  : %d \n",mydata.count);

	del_timer_sync(&mydata.timer);
	howmany[0] = 0;

	mydata.timer.expires = jiffies + (1 * HZ);
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = kernel_timer_blink;

	add_timer(&mydata.timer);
	return 1;
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
	printk("%d\n", howmany[0]);
	device_usage = 0;
	del_timer_sync(&mydata.timer);
	kfree(howmany);

	unregister_chrdev(major, IOM_DEVICE_NAME);
}

module_init(iom_device_init);
module_exit(iom_device_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
