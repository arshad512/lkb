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
#include <linux/interrupt.h>	// interrupt handling

/*
 * 	# cat /proc/interrupts | grep my_interrupt
 	19:       1153   IO-APIC  19-fasteoi   enp0s3, my_interrupt
	#
 *
 */

/*
 * Interrupt is when a device tell the os that
 * there is a change in state. This is done through
 * a handler which is registerd. And every time an
 * interrupt is triggered (raised) this handler 
 * is called.
 */


/*
 * See above output 19 IRQ is tied to networkcard.
 * "my_interupt" is comman name given to interupt.
 */
#define SIRQ 19
static int irq = SIRQ, my_dev_id; //, irq_counter = 0;

irqreturn_t my_interrupt (int irq, void *dev_id)
{
	printk(KERN_INFO "inside interrupt\n");
	/* Just observing, IRQ_HANDLE is to finish handling. */
	return IRQ_NONE;
}


static int __init code_init(void)
{
	printk(KERN_INFO "***** Hello world! Interrupt ***** \n");
	/*
	 * Set up interrupt handler. Everytime an interrupt is raised
	 * function my_interrupt() callback is called.
	 *
	 * Flag IRQF_SHARED means that interrupt line can be shared
	 * by more than one device.
	 */
	if (request_irq(irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id)) {
		printk(KERN_INFO "request_irq error %d\n", irq);
		return -EIO;
	}
	/* Non-zero return means that the module couldn't be loaded. */
	return 0;
}

static void __exit code_cleanup(void)
{
	printk(KERN_INFO "Cleaning up module.\n");
	/* Wait for interrupts to be completed before returning.*/
	synchronize_irq(irq);
	/* Remove the handler.  And disable the line if last one */
	free_irq(irq, &my_dev_id);
}

module_init(code_init);
module_exit(code_cleanup);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("interrupt example");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");
