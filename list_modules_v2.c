#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>

struct module *m, *m1;
struct list_head *list;

static int __init list_modules_v2(void)
{
	printk(KERN_INFO "Printing the modules\n");

       
	list_for_each(list, &m->list) {
		if (list == NULL)
			break;
		m1 = list_entry(list, struct module, list);
		printk(KERN_INFO "name %s", m1->name);
	}
	return 0;
}

static void __exit list_modules_v2_clean(void)
{
	printk(KERN_INFO "completed prinitng\n");
}

module_init(list_modules_v2);
module_exit(list_modules_v2_clean);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("sameeruddin shaik");

