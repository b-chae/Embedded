#ifndef _DRIVER_HEADER_H_
#define _DRIVER_HEADER_H_

/* libraries */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>

/* function declaration*/
static int inter_major=242, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;
static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static void timer_func(unsigned long timeout);
static void quit_func(unsigned long timeout);

irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler2(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler3(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg);

static inter_usage=0;

/* physical address*/
#define IOM_FND_ADDRESS 0x08000004

static unsigned char *iom_fpga_fnd_addr;

wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

/* fnd 정보 저장하는 structure */
static struct timer_data{
	struct timer_list timer;
	int time;
};

/* 3초 후 프로그램을 종료시키기 위한 timer structure */
static struct quit_data{
	struct timer_list timer;
	int quit_flag;
};

struct timer_data mydata;
struct quit_data quit_timer;

static struct file_operations inter_fops =
{
	.open = inter_open,
	.write = inter_write,
	.release = inter_release,
};

#endif
