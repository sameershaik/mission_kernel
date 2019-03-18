#ifndef __CHAR_DRIVER__
#define __CHAR_DRIVER__
#define CC 2000
#define CS 500

struct data_capsule {
	void **data;
	struct data_capsule *next;
};
struct char_device {
	struct data_capsule *data;
	int capsule_capacity;
	int capsule_strength;
	size_t size;
	struct cdev cdev;
};

#endif
