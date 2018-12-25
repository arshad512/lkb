#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>  /* error codes */
#include <linux/sched.h>
#include <linux/mm.h> /* find_vma */
#include <linux/mm_types.h> /* vm_area_struct */
#include <linux/sched/signal.h> /* for_each_process */
#include <linux/sched/task_stack.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple task_struct sample to print all task");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

static int __init start(void)
{
	/*
	 * Kernel represents process/task as an instance
	 * of task_struct
	 */
	struct task_struct *task;

	/* get global current pointer :- Points to current task */
	task = current;
	
	pr_info("Hello, World!\n");

	/*
	 * loop through each task and print task name and pid
	 */
	for_each_process(task) {
  		pr_info("process: %s, PID: %d", task->comm, task->pid);
	}	
		
	return 0;
}

static void __exit stops(void) {
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(start);
module_exit(stops);

