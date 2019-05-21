#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>

static int __init
perfprofile_init(void)
{
	printk(KERN_ALERT "load perfprofile module\n");
	return 0;
}

static void __exit
perfprofile_exit(void)
{
	printk(KERN_ALERT "exit perfprofile module\n");
}

module_init(perfprofile_init);
module_exit(perfprofile_exit);
MODULE_LICENSE("GPL");
