#ifndef __CHAR_DRIVER__
#define __CHAR_DRIVER__

struct char_device {
	char message[1024];
	ssize_t buf_size ;
	unsigned long block_size;
	struct cdev cdev;
};

#endif
