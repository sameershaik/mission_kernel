#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/device.h>

static int my_char_major = 0;
static int my_char_minor = 0;

static dev_t dev;
static struct class *my_char_class;
static struct device *dev_ret;
static struct cdev cdev;

static int char_dev_open(struct inode *ind, struct file *filp)
{
	printk(KERN_INFO"Driver open is called\n");
	return 0;
}

static int char_dev_close(struct inode *ind, struct file *filp)
{
	printk(KERN_INFO"Driver close is called\n");
	return 0;
}

static ssize_t char_dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO"Driver read is called\n");
	return 0;
}

static ssize_t char_dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO"Driver write is called\n");
	return 0;
}


struct file_operations my_char_fops = {
	.open = char_dev_open,
	.release = char_dev_close,
	.read = char_dev_read,
	.write = char_dev_write,
};

		
	
static int __init char_init(void)
{
	int res;
	
	if (my_char_major) {
		dev = MKDEV(my_char_major, my_char_minor);
		res = register_chrdev_region(dev, 1, "my_char_driver");
	} else {
		res = alloc_chrdev_region(&dev, my_char_minor, 0, "my_char_driver");
		my_char_major = MAJOR(dev);
	}
	if (res < 0) {
		printk(KERN_INFO"My char driver didn't got the major number\n");
		return res;
	}
	printk(KERN_INFO"My char driver major is %d \n", my_char_major);

	my_char_class = class_create(THIS_MODULE, "char");
	
	if (IS_ERR(my_char_class)) {
		unregister_chrdev_region(my_char_major, 1);
		return PTR_ERR(my_char_class);
	}

	dev_ret = device_create(my_char_class, NULL, dev, NULL, "my_char_dev");

	if (IS_ERR(dev_ret)) {
		unregister_chrdev_region(my_char_major, 1);
		return PTR_ERR(dev_ret);
	}

	cdev_init(&cdev, &my_char_fops);

	if ((res = cdev_add(&cdev, my_char_major, 1)) < 0) {
		device_destroy(my_char_class, my_char_major);
		class_destroy(my_char_class);
		unregister_chrdev_region(my_char_major, 1);
		return res;
	}

	return 0;
	
	
}

static void __exit char_clean(void)
{
	cdev_del(&cdev);
	device_destroy(my_char_class, my_char_major);
	class_destroy(my_char_class);
	unregister_chrdev_region(my_char_major, 1);
	printk(KERN_INFO"Bybye\n");
}

module_init(char_init);
module_exit(char_clean);
  
MODULE_LICENSE("GPL");
