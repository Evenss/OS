#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/moduleparam.h>

static int PID;

module_param(PID,int,0644);
 

static int findThread_init(void)
{
	struct task_struct *task,*task2;
	struct list_head *list,*list2;
	printk(KERN_ALERT"You will find No.%d process\n",PID);

	if(task=pid_task(find_vpid(PID),PIDTYPE_PID)){	//find this pid
		printk(KERN_ALERT"Parent:%s %d\n",
			(task->parent)->comm,(task->parent)->pid);//output it's parents

		printk(KERN_ALERT"Childern:Name\tPID\n");//ouput it's childern
		list_for_each(list,&(task->children)){
			task2=list_entry(list,struct task_struct,sibling);
			printk(KERN_ALERT"%s\t%d\n",task2->comm,task2->pid);
		}

		printk(KERN_ALERT"\nBrother:Name\tPID\n");//output it's brother
		list_for_each(list2,&(task->real_parent->children)){
			task2=list_entry(list2,struct task_struct,sibling);
			printk(KERN_ALERT"%s\t%d\n",task2->comm,task2->pid);
		}
		printk("\n");
	}
	else{//no this pid
		printk(KERN_ALERT"There is not No.%d process!\n",PID);
	}

	return 0;
}


static void findThread_exit(void)
{
	printk(KERN_ALERT"This is end!\n");
}

module_init(findThread_init);
module_exit(findThread_exit);

MODULE_LICENSE("GPL");
