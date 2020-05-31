#include "driver_header.h"
#include "write.h"

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg) {
	printk(KERN_ALERT "volume up! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	
	del_timer_sync(&mydata.timer);
	
	mydata.time = 0;
	fnd_write(0);
	
	printk("timer start\n");
	
	return IRQ_HANDLED;
}

irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg) {
        printk(KERN_ALERT "back! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 12)));
        return IRQ_HANDLED;
}

irqreturn_t inter_handler3(int irq, void* dev_id,struct pt_regs* reg) {
	printk(KERN_ALERT "home! = %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));
	
	del_timer_sync(&mydata.timer);

	mydata.timer.expires = get_jiffies_64() + 1*HZ;
	mydata.timer.function = timer_func;
	mydata.timer.data = (unsigned long)&mydata;
	
	add_timer(&mydata.timer);
	printk("timer start\n");
    return IRQ_HANDLED;
}

irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg) {
        printk(KERN_ALERT "volume down! = %x\n", gpio_get_value(IMX_GPIO_NR(5, 14)));

        __wake_up(&wq_write, 1, 1, NULL);
		//wake_up_interruptible(&wq_write);
		printk("wake up\n");
		
		del_timer_sync(&mydata.timer);
		mydata.time = 0;
		fnd_write(0);

        return IRQ_HANDLED;
}


static int inter_open(struct inode *minode, struct file *mfile){
	if(inter_usage != 0){
		return -EBUSY;
	}
	
	inter_usage = 1;

	int ret;
	int irq;

	printk(KERN_ALERT "Open Module\n");

	// int1
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler1, IRQF_TRIGGER_RISING, "home", 0);

	// int2
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler2, IRQF_TRIGGER_RISING, "back", 0);

	// int3
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler3, IRQF_TRIGGER_RISING, "volup", 0);

	// int4
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler4, IRQF_TRIGGER_RISING, "voldown", 0);

	return 0;
}

static int inter_release(struct inode *minode, struct file *mfile){
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	
	printk(KERN_ALERT "Release Module\n");
	
	inter_usage = 0;
	
	return 0;
}

static void timer_func(unsigned long timeout){
	struct timer_data *p_data = (struct timer_data*)timeout;
	
	printk("timer func %d\n", p_data->time);
	p_data->time = (p_data->time + 1)%3600;
	
	mydata.timer.expires = get_jiffies_64() + 1*HZ;
	mydata.timer.data = (unsigned long)&mydata;
	mydata.timer.function = timer_func;
	
	add_timer(&mydata.timer);
	
	fnd_write(mydata.time);
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
	fnd_write(0);
    printk("sleep on\n");
	interruptible_sleep_on(&wq_write);
	return 0;
}

static int inter_register_cdev(void)
{
	int error;
	if(inter_major) {
		inter_dev = MKDEV(inter_major, inter_minor);
		error = register_chrdev_region(inter_dev,1,"stopwatch");
	}else{
		error = alloc_chrdev_region(&inter_dev,inter_minor,1,"stopwatch");
		inter_major = MAJOR(inter_dev);
	}
	if(error<0) {
		printk(KERN_WARNING "stopwatch: can't get major %d\n", inter_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", inter_major);
	cdev_init(&inter_cdev, &inter_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &inter_fops;
	error = cdev_add(&inter_cdev, inter_dev, 1);
	if(error)
	{
		printk(KERN_NOTICE "inter Register Error %d\n", error);
	}
	return 0;
}

static int __init inter_init(void) {
	int result;
	if((result = inter_register_cdev()) < 0 )
		return result;
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/stopwatch, Major Num : 242 \n");
	
	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	
	init_timer(&mydata.timer);
	return 0;
}

static void __exit inter_exit(void) {
	iounmap(iom_fpga_fnd_addr);
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	printk(KERN_ALERT "Remove Module Success \n");
	inter_usage = 0;
	del_timer_sync(&mydata.timer);
}

module_init(inter_init);
module_exit(inter_exit);
	MODULE_LICENSE("GPL");
