#ifndef _IOCTL_H_
#define _IOCTL_H_

#include <linux/ioctl.h>

#define IOCTL_TYPE 'i'

#define IOCTL_START _IO(IOCTL_TYPE, 1)

#define IOCTL_SEND_ARG _IOW(IOCTL_TYPE, 0, struct timer_option)

#endif
