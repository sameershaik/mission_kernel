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

static struct class *char_class = NULL;
static struct device *char_dev = NULL;

static int char_major = 0;
static int char_minor = 0;

static dev_t dev;

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
struct data_capsule *char_follow(struct char_device *dev, int n)
{
	struct data_capsule *dc = dev->data;

	if (!dc) {
		dc = dev->data = kmalloc(sizeof(struct data_capsule), GFP_KERNEL);
		if (dc == NULL)
			return NULL;
		memset(dc, 0, sizeof(struct data_capsule));
	}
	while (n--) {
		if (!dc->next) {
			dc->next = kmalloc(sizeof(struct data_capsule), GFP_KERNEL);
			if (dc->next == NULL)
				return NULL;
			memset(dc->next, 0, sizeof(struct data_capsule));
		}
		dc = dc->next;
		continue;
	}
	return dc;
}
static ssize_t char_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct char_device *cl_dev;
	cl_dev = (struct char_device *) filp->private_data; 
	struct data_capsule *dc;
	int cc = cl_dev->capsule_capacity, cs = cl_dev->capsule_strength;
	int itemsize = cc * cs;
	int item, s_pos, q_pos, rest;
	ssize_t ret = 0;
	*f_pos = 0;
	
	printk("The count is %d, offset is %d", count, *f_pos);

	
	if (*f_pos >= cl_dev->size) {
		printk(KERN_WARNING" offest is out of bounds\n");
		return ret;
	}

	if (*f_pos + count > cl_dev->size) 
		count = cl_dev->size - *f_pos;

	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / cc;
	q_pos = rest % cc;

	dc = char_follow(cl_dev, item);

	if (dc == NULL || ! dc->data || !dc->data[s_pos])
		return -1;

	if (count > cc - cs)
		count = cc - cs;
	if (copy_to_user(buf, dc->data[s_pos], count)) {
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
	struct data_capsule *dc;
	int cc = cl_dev->capsule_capacity, cs = cl_dev->capsule_strength;
	int itemsize = cc * cs;
	int item, s_pos, q_pos, rest;
	ssize_t ret = 0;

	printk("The count is %d, offset is %d", count, *f_pos);

	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / cc;
	q_pos = rest % cc;

	dc = char_follow(cl_dev, item);

	if (dc == NULL) {
		printk("Data capsule is NULL\n");
		return -1;
	}
	if (!dc->data) {
		dc->data = kmalloc(cs * sizeof(char *), GFP_KERNEL);
		if (!dc->data)
			return -ENOMEM;
		memset(dc->data, 0, cs* sizeof(char *));
	}

	if(!dc->data[s_pos]) {
		dc->data[s_pos] = kmalloc(cc, GFP_KERNEL);
		if (!dc->data[s_pos])
			return -ENOMEM;
	}

	if (count > cc - cs)
		count = cc - cs;
	
	
	if(copy_from_user(dc->data[s_pos], buf, count))
	{
		ret = -EFAULT;
		return ret;
	}

	
	
	*f_pos += count;
	printk("The f_pos is %d\n", *f_pos);
	ret = count;

	if (cl_dev->size < *f_pos)
		cl_dev->size = *f_pos;

	return ret;
		
}

loff_t char_seek(struct file *fp, loff_t off, int whence)
{
	struct char_device *dev = fp->private_data;
	loff_t newposs;
	switch(whence) {
	case 0:
		newposs = off;
		break;
	case 1:
		newposs = fp->f_pos + off;
		break;
	case 2:
		newposs = dev->size + off;
		break;
		
	default:
		return -EINVAL;
	}
	if (newposs < 0)
		return -EINVAL;
	fp->f_pos = newposs;
	return newposs;
}

struct file_operations my_char_fops = {
	.open = char_open,
	.read = char_read,
	.write = char_write,
	.release = char_release,
	.llseek = char_seek
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
	my_dev->capsule_capacity = CC;
	my_dev->capsule_strength = CS;
	res = cdev_add(&my_dev->cdev, devno, 1);

	if (res) {
		printk(KERN_WARNING" UNABLE TO ADD THE CHAR DEVICE\n");
		return res;
	}
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


