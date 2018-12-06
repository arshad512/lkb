#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>  /* error codes */
#include <linux/sched.h> /* task states (TASK_INTERRUPTIBLE and friends ) */
#include <linux/mm.h> /* find_vma */
#include <linux/mm_types.h> /* vm_area_struct */
#include <linux/workqueue.h>
#include <linux/delay.h> /* msleep() */
#include <linux/time.h>
#include <linux/wait.h> /* wait_event */
#include <linux/workqueue.h>
#include <linux/slab.h>     /* for kmalloc() */
#include <linux/fs.h>		// for basic filesystem
#include <linux/proc_fs.h>	// for the proc filesystem
#include <linux/seq_file.h>	// for sequence files
#include <linux/jiffies.h>	// for jiffies

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("work queues :- Global/Shared");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * Work queue
 *
 * Work queue allows to defer some work to be executed in future based on
 * some condition or action. Work queues are build on top of kernel thread.
 *
 * There are two type of Work queue:-
 * 1.	Default or shared or global queue: This handler is set of kernel thread
 * 	-	Should not sleep. As other task scheduled to run on the
 * 		queue would not run until sleep is woken up.
 * 	-	Slower as we do not know with whom we are sharing the queue.
 * 		Task which is scheduled to run on this is always slower.
 *
 * 2.	Dedicated kernel thread: This handler is used when ever a dedicated
 * 	queue handler is needed.
 *
 * Unlike softirq Work queue can sleep in bottom half and can hold mutex.
 *
 * This kernel code will only deal with work queue of type 1.
 */

static int condition = 0;

/*
 * wait_queue_head_t ---->[	|	|	|...
 * 			    ^       ^       ^
 * 			    |       |       |
 * 			  Work    Work    Work (Work is of type work_struct)
 */

struct work_struct_data {
	/*
	 * Single unit of work is defined as struct "work_struct"
	 * struct work_struct:- Unit of work scheduled to run at later time.
	 * and...
	 * sturct delayed_work:- Unit of work scheduled to run after atleast some time delay.
	 */
	struct work_struct unit_of_work;
	/*
	 * Represents the queue as a whole.
	 * It is the head of the waiting queue.
	 */
	wait_queue_head_t my_wq;
	int data;
};

static void do_work_handler(struct work_struct *work)
{
	struct work_struct_data *my_data = container_of(work, \
				 struct work_struct_data, unit_of_work);
	pr_info("Inside work handler..., value =  %d\n", my_data->data);
	/* force sleep for 5 seconds */
	msleep(5000);
	/* force condition to be true */
	condition = 5;
	/*
	 * Task is put to sleep using wait_event_interruptible()
	 * Wake up the task using wake_up_interruptible().
	 * If condition not met, task will remain sleeping forever.
	 */
	wake_up_interruptible(&my_data->my_wq);
	kfree(my_data);
}

static int __init start(void)
{
	struct work_struct_data * my_data;

	my_data = kmalloc(sizeof(struct work_struct_data), GFP_KERNEL);
	if (!my_data) {
		pr_info("kmalloc failed\n");
		return -1;
	}
	/* Assign some value */
	my_data->data = 1400;

	INIT_WORK(&my_data->unit_of_work, do_work_handler);
	init_waitqueue_head(&my_data->my_wq);

	/* Put work into global workqueue */
	schedule_work(&my_data->unit_of_work);
	pr_info("I'm going to sleep now ...Value of condition = %d\n",condition);

	/*
	 * Sleep until condition is true. Will continue to sleep until value of condition is != 0
	 * Process is put in TASK_INTERRUPTIBLE.
	 * Condition is checked every time wait_queue is woken up using wake_up_interruptible()
	 */
	wait_event_interruptible(my_data->my_wq, condition != 0);
	printk("woken up...Value of condition = %d\n", condition);
	return 0;
}

static void __exit stops(void)
{
}


module_init(start);
module_exit(stops);

