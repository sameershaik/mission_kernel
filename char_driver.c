#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

static int my_char_major = 0;
static int my_char_minor = 0;

static dev_t dev;

struct my_char {
	struct cdev cdev;
	char buff[128];
	unsigned long size;
};

/* struct file_operations my_char_fops = { */
/* 	.OWNER = THIS_MODULE, */
/* 	.open = char_dev_open, */
/* 	.close = char_dev_close, */
/* 	.read = char_dev_read, */
/* 	.write = char_dev_write, */
/* 	.release = char_dev_release */
/* }; */

	
static int __init char_init(void)
{
	int res;
	printk(KERN_INFO"hello world \n");
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
	return 0;
}

static void __exit char_clean(void)
{
	unregister_chrdev_region(my_char_major, 1);
	printk(KERN_INFO"Bybye\n");
}

module_init(char_init);
module_exit(char_clean);
  
