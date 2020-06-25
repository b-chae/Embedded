#ifndef _DRIVER_HEADER_H_
#define _DRIVER_HEADER_H_

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <asm/io.h>

#define IOM_DEVICE_MAJOR 242
#define IOM_DEVICE_NAME "dev_driver"

/* physical address*/
#define IOM_FND_ADDRESS 0x08000004
#define IOM_LED_ADDRESS 0x08000016
#define IOM_FPGA_DOT_ADDRESS 0x08000210
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090

#define STRLEN_STUDENT_NUMBER 8
#define STRLEN_MY_NAME 11

static int device_usage = 0;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;

const char* student_number = "20171696";
const char* my_name = "byeori chae";

#include "fpga_dot.h"

/* function declaration*/
int iom_device_open(struct inode *, struct file *);
int iom_device_release(struct inode *, struct file *);
ssize_t iom_device_write(struct file *, const char *, size_t, loff_t *);
void update_data(void);
void clear_device(void);
void deal_with_data(void);
static void timer_func(unsigned long timeout);
void fnd_write(int n, int index);
void dot_write(int n);
void led_write(unsigned char n);
void text_write(int, int);
static long iom_device_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations iom_device_fops =
{ .open = iom_device_open, .write = iom_device_write,
	.release = iom_device_release, .unlocked_ioctl = iom_device_ioctl, };

/* struct struct_mydata
 * timer 함수에 전달할 정보를 가지고 있는 구조체
 * count : 현재 timer_func 실행 횟수
 * rotation_count : 로테이션 횟수(한 자리에서 모든 문양이 한 번씩 출력되는 것이 끝나면 0으로 초기화된다.)
 * current_num : 현재 출력하는 문양(1~8)
 * fnd_index : 현재 FND에 출력되는 위치(0~3)
 * text_index_i : 학번이 text lcd에 출력되는 위치 (0~15)
 * text_index_j : 내 이름이 text lcd에 출력되는 위치 (0~15)
 * i_direction : 현재 학번이 text lcd에서 움직이는 방향 (1이면 오른쪽 -1이면 왼쪽)
 * j_direction : 현재 이름이 text lcd에서 움직이는 방향 (1이면 오른쪽 -1이면 왼쪽)
 */
static struct struct_mydata {
	struct timer_list timer;
	int count;
	int rotation_count;
	int current_num;
	int fnd_index;
	int text_index_i;
	int text_index_j;
	int i_direction;
	int j_direction;
};

/* struct timer_option : 사용자 옵션을 저장하는 구조체
 * timer_interval : 1-100 HZ값(1~100)
 * timer_count : 디바이스 출력 변경 횟수(1~100)
 * timer_init : fnd에 출력되는 초기 문양과 위치(0001~8000)
 */
struct timer_option{
	int timer_interval;
	int timer_count;
	int timer_init;
};

struct struct_mydata mydata;
struct timer_option option;
int major;

#endif
