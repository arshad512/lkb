#include <linux/module.h>    	// included for all kernel modules
#include <linux/kernel.h>    	// included for KERN_INFO
#include <linux/init.h>      	// included for __init and __exit macros
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
#include <linux/interrupt.h>	// interrupt handling
#include <linux/workqueue.h>	// worke queue

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("work queues:- Dedicated");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * Work queue: Dedicated.
 *
 * Work queue allows to defer some work to be executed in future based on
 * some condition or action. Work queues are build on top of kernel thread.
 * Work queues provide a generic method to defer functionality to bottom halves.
 *
 * There are two type of Work queue:-
 * 1.>>>Default or shared or global queue: This handler is set of kernel thread
 * >>>>>->>>>>>>Should not sleep. As other task scheduled to run on the
 * >>>>>>>>>>>>>queue would not run until sleep is woken up.
 * >>>>>->>>>>>>Slower as we do not know with whom we are sharing the queue.
 * >>>>>>>>>>>>>Task which is scheduled to run on this is always slower.
 *
 * 2.>>>Dedicated kernel thread: This handler is used when ever a dedicated
 * >>>>>queue handler is needed.
 *
 * Unlike softirq Work queue can sleep in bottom half and can hold mutex.
 *
 * This kernel code will only deal with work queue of type 2. 
 *
 *
 * Workflow:
 *
 * Create link list in work queue. 
 * After every 14 creation flush.
 * If driver is 'rmmod' in between - flush at the exit 
 *
 * Interrupt handler will raise an interrupt and function my_interrupt will be called.
 * Inside function my_interrupt() we add the "work" to the dedicated queue. This will be processed later
 * We associate work_handler() function with the queue. 
 * 	- Inside work_handler() until moving_val is < 14; create a node and add to list.
 * 	- Else reset moving_val and print the contents of the list.
 */

#define SIRQ 19 // See above example 19 IRQ is tied to network card. "my_interrupt" is common name given to interrupt.
static int irq = SIRQ, my_dev_id; //, irq_counter = 0;
static int moving_val=0;
struct data *f[14];

/*
 * Decicated Queue:
 *
 * workqueue_struct ---->[     |       |       |...
 *                          ^       ^       ^
 *                          |       |       |
 *                        Work    Work    Work (Work is of type work_struct)
 *
 * Here work has a handler associated with itself, and this handler is executed
 * as part of bottom half or delayed execution.
 */

/*
 * Dedicated queue. Represented as struct workqueue_struct
 */
struct workqueue_struct *wq;
/*
 * Single unit of work is defined as struct "work_struct".
 * It has a function handler associated with it.
 *
 * struct work_struct:- Unit of work scheduled to run at later time.
 * and...
 * sturct delayed_work:- Unit of work scheduled to run after atleast some time delay.
 */
struct work_struct work;
void work_handler(struct work_struct *work);
void print_list(void);

LIST_HEAD(head);

struct data {
	int value;
	struct list_head next;
};

int mfoo(void *);

irqreturn_t my_interrupt (int irq, void *dev_id) 
{
	/* 
	 * Queue the work in the queue. Do not process it immediately.
	 * There is another API queue_delayed_work(), which also does
	 * the same work except it adds into queue after a delay
	 */
	queue_work(wq, &work);
	/* Just observing, IRQ_HANDLE is to finish handling */
	return IRQ_NONE;
}


static int __init start(void)
{
	pr_info("***** Hello world! ***** \n");

	/*
	 * Set up interrupt handler. Every time an interrupt is raised
	 * function my_interrupt() callback is called.
	 *
	 * Flag IRQF_SHARED means that interrupt line can be shared
	 * by more than one device.
	 */
	if (request_irq(irq, my_interrupt, IRQF_SHARED, "my_interrupts", &my_dev_id)) { 
		printk(KERN_INFO "request_irq error %d\n", irq);
		return -EIO;
	}
	/* 
	 * Initialize a workqueue.
	 * Create single threaded workqueue.
	 *
	 * Other API(macro) create_workqueue() will create
	 * workqueue on all the CPU.
	 */
	wq = create_singlethread_workqueue("mydrv");
	/* 
	 * Associate a handler.
	 */
	INIT_WORK(&work, work_handler);
	return 0;
}

static void __exit stops(void)
{
	/* Wait for interrupts to be completed before returning.*/
	synchronize_irq(irq);
	/* Remove the handler. And disable the line if last one */
	free_irq(irq, &my_dev_id);
	/*
	 * forces flush of workqueues. This call blocks.
	 * For shared workqueue. This should always be called.
	 */
	flush_workqueue(wq);
	/* flush and then clean up workqueue */
	destroy_workqueue(wq);
	printk(KERN_INFO "Cleaning up module.\n");

}

/*
 * defer work. Bunch up in link list and then print and flush
 */
void
work_handler(struct work_struct *work)
{
	int i = 0;
	spinlock_t sl;

	/* initialization lock */
	spin_lock_init(&sl);

	/* Create link list here */
	spin_lock(&sl);
	if (moving_val < 14 ) {
		f[moving_val] = kmalloc(sizeof(struct data), GFP_KERNEL);
		if (!f[moving_val]) {
			pr_info("kmalloc failed %d\n", moving_val);
			return;
		} else {
			pr_info("kmalloc passed for  %d\n", moving_val);
		}
		f[moving_val]->value = moving_val;
		list_add(&f[moving_val]->next, &head);

		moving_val++;
	} else {
		/* flush and print */
		moving_val = 0;
		print_list();
		/* delete old list */
		for (i=0;i<14;i++) {
			pr_info("deleting for  %d\n", i);
			list_del(&f[i]->next);
		}
	}
	spin_unlock(&sl);
}

void print_list(void)
{
	struct data *d;

	printk("***** Flush *****\n");
	list_for_each_entry(d, &head, next) {
		pr_info("link list = %d\n", d->value);
	}
}

module_init(start);
module_exit(stops);
