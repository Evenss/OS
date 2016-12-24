#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>//use for_each_process()

static int thread_init(void)
{
	struct task_struct *p;
	printk(KERN_ALERT"Name\tNumber\tState\tPrio\t");
	for_each_process(p)
	{
		printk(KERN_ALERT"%s\t%d\t%d\t%d\n",p->comm,p->pid,p->state,p->normal_prio);
	}
	return 0;
}

static void thread_exit(void)
{
	printk(KERN_ALERT"This is all !!\n");
}

module_init(thread_init);
module_exit(thread_exit);

MODULE_LICENSE("GPL"); 

