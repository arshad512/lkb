#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>	// proc entry.
#include <linux/seq_file.h>	// proc entry.
#include <linux/fs.h>		// simple_read_from_buffer
#include <linux/uaccess.h>	// copy_from_user
#include <linux/delay.h>	// mdelay etc...
#include <linux/sched.h>	// wait_queue
#include <linux/wait.h>		// wait_event
#include <linux/kthread.h>	// kernel thread
#include <linux/err.h>		// IS_ERR

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wait queues");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

#define SLEEP 2000

int mfoo(void *);
/*
 * When a process is waiting on external event. It is removed from run-queue and put into wait-queue until that event is met.
 * Insert process into wait-queue through a function add_wait_queue(), this is exclusive (any time event is met this process
 * will be woken up). Non-Exclusive on other hand is event not shared by any process
 */

/* Defines a task within kernel */
struct task_struct *mytask;
/*
 * Represents the queue as a whole.
 * It is the head of the waiting queue.
 */
wait_queue_head_t wq;
int condition = 0;

static int __init start(void)
{
	/* Initilize a wait queue */
	init_waitqueue_head(&wq);

	/* Create a kernel thread. Thread created will be in stopped state */
	mytask = kthread_create(mfoo, NULL, "my_super_task");
	if (IS_ERR(mytask)) {
		printk(KERN_INFO "kthread failed\n");
		return -1;
	}
	printk(KERN_INFO "***** Hello world! waiting...***** \n");

	/* Run task - Task is initialized in stopped state - We have to explicitly make it into run state */
	wake_up_process(mytask);

	/* give time for process to wake up and run. - kthread_Create creates it as stopped state */
	msleep(SLEEP);

	/* condition forced as false. Task will wake up on when condition != 0 */
	condition = 0;

	printk(KERN_INFO "***** Attempt 1: The condition is not met... ***** \n");
	/*
	 * Wake up from wait queue. Done through wake_up_interruptible().
	 * This will bring back process in wait queue to running queue only if the condition is met.
	 * Since the condition is not met, this will take an attempt but put process back into wait queue.
	 */
	wake_up_interruptible(&wq); 

	/* give time for process to wake up and run. - kthread_Create creates it as stopped state */
	msleep(SLEEP);

	/* condition forced as true. Task will wake up on when condition != 0 */
	condition = 1;

	printk(KERN_INFO "***** Attempt 2: The condition is met... ***** \n");
	/* Wake up from wait queue. - Attempt 2 */
	wake_up_interruptible(&wq);

	printk(KERN_INFO "***** wake up Done... 2nd time.... pause then exit***** \n");
	
	msleep(SLEEP); // pause then exit... 
	printk(KERN_INFO "***** Hello world! Done...***** \n");

	return 0;
}

static void __exit stops(void)
{	
	printk(KERN_INFO "Cleaning up module.\n");
}

int mfoo(void *arg) {
	printk(KERN_INFO "My thread is running...Oh its going to sleep... \n");

	/* Wait until condition gets true - or else put it back to sleep. */
	wait_event_interruptible(wq, condition > 0);
	
	printk(KERN_INFO "My thread is running again...\n");

	return 0;
}

module_init(start);
module_exit(stops);
