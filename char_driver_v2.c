#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "char_driver.h"
#include <linux/types.h>
#include <linux/device.h>

#define DEV_NAME "my_char_driver"
#define CLS_NAME "my_char_class"



static unsigned long buf_size;
static struct class *char_class = NULL;
static struct device *char_dev = NULL;

static int char_major = 0;
static int char_minor = 0;

static dev_t dev;

module_param(buf_size, long, 0);

static struct char_device *my_dev;

static int char_open(struct inode *inode, struct file *filp)
{
	struct char_device *cl_dev;

	cl_dev = container_of(inode->i_cdev, struct char_device, cdev);

	filp->private_data = cl_dev;
	
	return 0;
}

static int char_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t char_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct char_device *cl_dev;
	cl_dev = (struct char_device *) filp->private_data; 

	ssize_t ret = 0;
	*f_pos = 0;
	
	printk("The count is %d, offset is %d", count, *f_pos);

	if (*f_pos >= cl_dev->buf_size) {
		printk(KERN_WARNING" offest is out of bounds\n");
		return ret;
	}

	if (*f_pos + count > cl_dev->buf_size) 
		count = cl_dev->buf_size - *f_pos;
	
	if (copy_to_user(buf, &(cl_dev->message[*f_pos]), count) != 0) {
		ret = -EFAULT;
		return ret;
	}

	*f_pos += count;
	printk("The f_pos is %d\n", *f_pos);
	ret = count;
	
	return ret;
}
static ssize_t char_write(struct file *filp, const char *__user buf, size_t count, loff_t *f_pos)
{
	struct char_device *cl_dev;
	cl_dev = (struct char_device *) filp->private_data; 

	ssize_t ret = 0;

	printk("The count is %d, offset is %d", count, *f_pos);

	if (!cl_dev->message) {
		cl_dev->message = (void *)kmalloc(buf_size * sizeof(char), GFP_KERNEL);
		if (!cl_dev->message)
			return -ENOMEM;
		memset(cl_dev->message, 0, buf_size*(sizeof(char)));
	}
	
	if (*f_pos >= cl_dev->buf_size) {
		printk(KERN_WARNING" offest is out of bounds\n");
		return ret;
	}

	if (*f_pos + count > cl_dev->buf_size) 
		count = cl_dev->buf_size - *f_pos;


	if(copy_from_user(cl_dev->message[*f_pos], buf, count) != 0)
	{
		ret = -EFAULT;
		return ret;
	}

	
	
	*f_pos += count;
	printk("The f_pos is %d\n", *f_pos);
	ret = count;

	return ret;
		
}

struct file_operations my_char_fops = {
	.open = char_open,
	.read = char_read,
	.write = char_write,
	.release = char_release
};

	
static int  char_init(void)
{
	int res;
	dev_t devno;
	
	
	printk(KERN_INFO"hello world \n");
	if (char_major) {
		dev = MKDEV(char_major, char_minor);
		res = register_chrdev_region(dev, 1, DEV_NAME);
	} else {
		res = alloc_chrdev_region(&dev, char_minor, 0, DEV_NAME);
		char_major = MAJOR(dev);
	}
	if (res < 0) {
		printk(KERN_INFO"My char driver didn't got the major number\n");
		return res;
	}
	printk(KERN_INFO"My char driver major is %d \n", char_major);

	char_class = class_create(THIS_MODULE, CLS_NAME);

	if (IS_ERR(char_class)) {
		unregister_chrdev_region(char_major, 1);
		printk(KERN_ALERT"failed to register char class");
		return PTR_ERR(char_class);
	}

	my_dev = (struct char_device *)kzalloc(sizeof(struct char_device), GFP_KERNEL);

	if (my_dev == NULL) {
		res = -ENOMEM;
		printk(KERN_ALERT"no memory allocated for the device\n");
		return res;
	}
		
	devno = MKDEV(char_major, char_minor);

	cdev_init(&my_dev->cdev, &my_char_fops);

	my_dev->cdev.owner = THIS_MODULE;
	my_dev->cdev.ops = &my_char_fops;

	res = cdev_add(&my_dev->cdev, devno, 1);

	if (res) {
		printk(KERN_WARNING" UNABLE TO ADD THE CHAR DEVICE\n");
		return res;
	}
	my_dev->buf_size = buf_size;
	char_dev = device_create(char_class, NULL, devno, NULL, DEV_NAME);

	if (IS_ERR(char_dev)) {
		printk(KERN_ALERT"unable to create the charatcer device\n");
		cdev_del(&my_dev->cdev);
		class_destroy(char_class);
		unregister_chrdev_region(char_major, 1);
		return PTR_ERR(char_dev);
	}


	return 0;
	
}

static void  char_clean(void)
{
	cdev_del(&my_dev->cdev);
	device_destroy(char_class, dev);
	class_unregister(char_class);
	class_destroy(char_class);
	unregister_chrdev_region(char_major, 1);
	kfree(my_dev);
	printk(KERN_INFO"Bybye\n");
}

module_init(char_init);
module_exit(char_clean);
  
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("a small char driver");


