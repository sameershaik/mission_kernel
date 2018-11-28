#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

static void list_modules(char *filename)
{
	int fd;
	char buf[128];
	
	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);
	
	fd = sys_open(filename, O_RDONLY, 0);

	if (fd >= 0) {
		printk(KERN_DEBUG);
		while (sys_read(fd, buf, 128) != 0 )
