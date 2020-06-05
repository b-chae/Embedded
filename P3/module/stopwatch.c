#include "driver_header.h"
#include "write.h"

/* volume up button pressed
 * return to initial state!
 * reset fnd time to 0000 and delete added timer
 */
irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg) {
	printk(KERN_ALERT "volume up! = %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
	
	del_timer(&mydata.timer);
	
	mydata.time = 0;
	mydata.flag = -1;
	clear_device();
	
	printk("timer reset\n");
	return IRQ_HANDLED;
}

/* back button pressed
 * pause for a while
 */
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg) {
        printk(KERN_ALERT "%x back button pressed\n", gpio_get_value(IMX_GPIO_NR(1, 12)));
	
	//if time is not stopped, change to pause state
	if(mydata.flag == 1){
		mydata.flag = 0;
		printk("pause!\n");
	}
        return IRQ_HANDLED;
}

/* home button pressed
 * start a timer!
 * update fnd time every a second 
 */
irqreturn_t inter_handler3(int irq, void* dev_id,struct pt_regs* reg) {
	printk(KERN_ALERT "%x home button pressed\n", gpio_get_value(IMX_GPIO_NR(2, 15)));
	
	//if initial state, start a new timer!
	if(mydata.flag == -1){
		mydata.flag = 1;
		del_timer(&mydata.timer);

		mydata.timer.expires = get_jiffies_64() + 1*HZ;
		mydata.timer.function = timer_func;
		mydata.timer.data = (unsigned long)&mydata;
		
		add_timer(&mydata.timer);
		printk("timer start\n");
	}//if not initial state, change to ongoing mode
	else if(mydata.flag == 0){
		mydata.flag = 1;
	}
    return IRQ_HANDLED;
}

/* 프로그램 종료
 * 값을 초기화 시키고 타이머 삭제
 */
static void quit_func(unsigned long timeout){
	mydata.time = 0;
	mydata.flag = -1;
	quit_timer.quit_flag = 0;
	clear_device();
	
	  __wake_up(&wq_write, 1, 1, NULL);
	//wake_up_interruptible(&wq_write);
	printk("wake up\n");
	
	del_timer(&mydata.timer);
}

/* volume down button pressed
 * if IRQF_TRIGGER_FALLING : start a quit_timer to quit the program(3초동안 유지되면 프로그램이 종료될 것)
 * if IRQF_TRIGGER_RISING : delete the quit_timer(프로그램 종료되지 않는다.)
 */
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg) {
	
	//버튼을 누른 경우 3초 후 프로그램 종료하도록 설정한다.
	if(quit_timer.quit_flag == 0 && gpio_get_value(IMX_GPIO_NR(5, 14)) == 0){
		printk(KERN_ALERT "%x volume down button pressed\n", gpio_get_value(IMX_GPIO_NR(5, 14)));
		quit_timer.quit_flag = 1;
		
		quit_timer.timer.expires = get_jiffies_64() + 3*HZ;
		quit_timer.timer.function = quit_func;
		quit_timer.timer.data = (unsigned long)&quit_timer;
		
		add_timer(&quit_timer);
		
	}//그러나 3초가 되기 전에 버튼을 뗀 경우 설정된 타이머를 제거한다. -> 3초 이상 연속해서 눌러야만 실행됨
	else if(gpio_get_value(IMX_GPIO_NR(5, 14)) == 1){
		printk(KERN_ALERT "volume down button released\n");
		quit_timer.quit_flag = 0;
		del_timer(&quit_timer.timer);
	}
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

	// home 버튼 interrupt 서비스 함수 등록
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler1, IRQF_TRIGGER_FALLING, "home", 0);

	// back 버튼 interrupt 서비스 함수 등록
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler2, IRQF_TRIGGER_FALLING, "back", 0);

	// vol+ 버튼 interrupt 서비스 함수 등록
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler3, IRQF_TRIGGER_FALLING, "volup", 0);

	// vol- 버튼 interrupt 서비스 함수 등록
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq, inter_handler4, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "voldown", 0);

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

/* 1초마다 fnd 출력값을 업데이트 */
static void timer_func(unsigned long timeout){
	
	if(mydata.flag == 1){
		struct timer_data *p_data = (struct timer_data*)timeout;
		
		p_data->time = (p_data->time + 1)%3600;
		printk("timer func %d\n", p_data->time);
		
		mydata.timer.expires = get_jiffies_64() + 1*HZ;
		mydata.timer.data = (unsigned long)&mydata;
		mydata.timer.function = timer_func;
		
		add_timer(&mydata.timer);
		
		fnd_write(mydata.time);
	}
	else if(mydata.flag == 0){
		mydata.timer.expires = get_jiffies_64() + 1*HZ;
		mydata.timer.data = (unsigned long)&mydata;
		mydata.timer.function = timer_func;
		
		add_timer(&mydata.timer);
	}
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
	clear_device();
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
	init_timer(&quit_timer.timer);
	
	mydata.time = 0;
	mydata.flag = -1;
	quit_timer.quit_flag = 0;
	return 0;
}

static void __exit inter_exit(void) {
	iounmap(iom_fpga_fnd_addr);
	cdev_del(&inter_cdev);
	unregister_chrdev_region(inter_dev, 1);
	printk(KERN_ALERT "Remove Module Success \n");
	inter_usage = 0;
	del_timer_sync(&mydata.timer);
	del_timer_sync(&quit_timer.timer);
}

module_init(inter_init);
module_exit(inter_exit);
	MODULE_LICENSE("GPL");
