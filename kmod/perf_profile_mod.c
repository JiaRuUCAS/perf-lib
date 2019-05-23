#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/file.h>
#include <linux/perf_event.h>
#include <linux/syscalls.h>

static u64 orig_cr0;
static unsigned long *syscall_table = 0;
static long (*orig_perf_event_open)(
				struct perf_event_attr __user *,
				pid_t, int, int, unsigned long);

asmlinkage long
sys_perfprofile_event_open(
				struct perf_event_attr __user *attr_uptr,
				pid_t pid, int cpu, int group_fd,
				unsigned long flags)
{
	printk(KERN_ALERT "perfprofile: new perf_event_open");
	return orig_perf_event_open(attr_uptr, pid, cpu, group_fd, flags);
}

static u64 __clear_and_get_cr0(void)
{
	u64 cr0 = 0;
	u64 ret;

	asm volatile ("movq %%cr0, %0" : "=a"(cr0));
	ret = cr0;

	/* clear the 20 bit of CR0, a.k.a WP bit */
	cr0 &= ~0x10000LL;

	asm volatile ("movq %0, %%cr0" :: "a"(cr0));
	return ret;
}

static void __recover_cr0(u64 val)
{
	asm volatile ("movq %0, %%cr0"
					:
					: "a"(val));
}

/* replacing the syscall during the initialization of the module */
static int __init perfprofile_init(void)
{
#if defined(SYSCALL_TBL_ADDR) && defined(SYS_PERF_ADDR)
	printk(KERN_ALERT "perfprofile: syscall_table %lx, "
					  "sys_perf_event_open %lx",
					  SYSCALL_TBL_ADDR, SYS_PERF_ADDR);

	// identify syscall table
	syscall_table = (unsigned long *)SYSCALL_TBL_ADDR;

	// check whether the address of syscall table is correct.
	if (syscall_table[__NR_perf_event_open]
					== (unsigned long)SYS_PERF_ADDR) {
		printk(KERN_ALERT "perfprofile: find syscall table\n");
	} else {
		printk(KERN_ALERT "perfprofile: cannot find syscall table\n");
		return -1;
	}

	// set function pointer to the original perf_event_open
	orig_perf_event_open = (void *)SYS_PERF_ADDR;

	// replace sys_perf_event_open
	orig_cr0 = __clear_and_get_cr0();
	syscall_table[__NR_perf_event_open]
			= (unsigned long)&sys_perfprofile_event_open;
	__recover_cr0(orig_cr0);

	printk(KERN_ALERT "perfprofile: module is loaded\n");
	return 0;

#else
	printk(KERN_ALERT "perfprofile: failed to load the module, "
					  "SYSCALL_TBL_ADDR and SYS_PERF_ADDR cannot"
					  " be found.");
	return -1;
#endif
}

static void __exit
perfprofile_exit(void)
{
	orig_cr0 = __clear_and_get_cr0();
	syscall_table[__NR_perf_event_open] = (unsigned long)SYS_PERF_ADDR;
	__recover_cr0(orig_cr0);

	printk(KERN_ALERT "perfprofile: module is removed\n");
}

module_init(perfprofile_init);
module_exit(perfprofile_exit);
MODULE_LICENSE("GPL");
