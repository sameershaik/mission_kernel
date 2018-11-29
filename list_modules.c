#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include "fs/internal.h"

struct file *file_open(const char *path, int flags, int rights)
{
	struct file *fp = NULL;
	struct cred *cred;
	mm_segment_t old_fs;
	int err = 0;

	old_fs = get_fs();
	set_fs(get_ds());
	vfs_open(path, fp, cred);
	set_fs(old_fs);
	
	if (IS_ERR(fp)) {
		err = PTR_ERR(fp);
		return NULL;
	}
	
	return fp;
}
static int list_modules(struct file *fp, unsigned long long offset, unsigned char *data, unsigned int size)
{

	int ret;	
	mm_segment_t old_fs = get_fs();

	set_fs(get_ds());
	ret = vfs_read(fp, data, size, &offset);
	
	set_fs(old_fs);
	return ret;
}

static int __init list_main(void)
{
	struct filep *filp;
	char buf[1024];
	int err;
  
	filp = file_open("/proc/modules", O_RDONLY, 0);

	err = list_modules(filp, 0, buf, 1024);

	if (err < 1024)
		printk(KERN_DEBUG"File reading error\n");

	printk(KERN_INFO "%s \n", buf);
  
  
	return 0;
}

static void __exit list_exit(void)
{
}

module_init(list_main);
module_exit(list_exit);
MODULE_AUTHOR("sameeruddin shaik");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("module to print the list of modules");

      

	       
