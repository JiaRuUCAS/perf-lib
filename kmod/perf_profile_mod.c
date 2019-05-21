#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>

static int __init
perfprofile_init(void)
{

#ifdef SYS_INIT_MODULE_ADDR
	printk(KERN_ALERT "load perfprofile module: sys_init_module 0x%lx\n",
				   SYS_INIT_MODULE_ADDR);
#else
	printk(KERN_ALERT "load perfprofile module\n");
#endif
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
