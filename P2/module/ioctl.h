#ifndef _IOCTL_H_
#define _IOCTL_H_

#define IOCTL_TYPE 'i'

#define IOCTL_START			_IO(IOCTL_TYPE, 1)

#define IOCTL_SEND_ARG		_IOW(IOCTL_TYPE, 1 , mydata)

#endif