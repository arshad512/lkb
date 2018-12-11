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

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("spinlock");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * Spinlock: Synchronization mechanism.
 *
 * Any task which acquire spinlock; will be active loop until 
 * lock is released. spinlock is cpu intensive.
 * If lock not taken, it will spin until lock is taken.
 *
 * Remember race can be triggered through
 * 1. Interrupt handler
 * 2. Preemption
 * 3. Multiple CPU
 * and proper alignment of these is necessary to get deterministic result.
 *
 * - preemption is disabled on the cpu where the code is running
 * 	when holding the spinlock. Task waiting(spin waiting) on 
 * 	other cpu, there preemption is not disabled.
 * - when preemption is enabled, no scheduling is allowed.
 * - spinning on a cpu means no other task can run on that processor.
 * - on single cpu spinlock is disabled.
 * - However, on single CPU spin_lock_irqsave() is used to disable
 *   	the interrupt. This prevents interrupt concurrency.
 * - On SMP spin_lock_irqsave() - disables interrupt on current cpu.
 *   	on other cpu interrupt is still active, and waiters would
 *   	spin to get the lock.
 *
 * Spinlock not to be called code which does kmalloc(x, GFP_KERNEL); as 
 * this sleeps.
 *
 * Dynamically
 * struct spinlock_t sl;
 * spin_lock_init(sl);
 * 
 */

static int __init start(void)
{
	spinlock_t sl;
	unsigned long flag;
	int i;

	printk(KERN_INFO "Start...!\n");

	/* initialization lock */
	spin_lock_init(&sl);

	/*
	 * Type 1: Locking between two user context process.
	 *
	 * Take lock or spin until lock is taken - non IRQ version
	 * Data only accessed from process context. No interrupt handler
	 * will ever touch this data
	 */
	spin_lock(&sl);

	/*
	 * check if lock is acquired ?
	 * Return 0 if lock is free and not acquired.
	 * Return !0 if lock is acquired
	 */
	i = spin_is_locked(&sl);
	if (i == 0) 
		printk(KERN_INFO "spin_is_lock:Lock is free");
	else
		printk(KERN_INFO "spin_is_lock:Lock is taken");

	/*
	 * Try to acquire lock only _ONCE_ and return, this will not spin.
	 * Return !0 if lock is obtained.
	 * Return 0 if cannot take lock. Or Lock already taken.
	 */
	i = spin_trylock(&sl);
	if (i == 0) 
		printk(KERN_INFO "spin_trylock:Lock already taken.");
	else
		printk(KERN_INFO "spin_trylock:Lock is free and lock taken");

	/* Release lock */
	spin_unlock(&sl);

	/*
	 * Type 2: Locking between user context and interrupt context
	 *
	 * If data accessed from 1)process and 2) interrupt context.
	 * 1. Take Lock 
	 * 2. Disable interrupt only on local CPU 
	 * 3. Store the state of interrupt in flag.
	 */
	spin_lock_irqsave(&sl, flag);

	/*
	 * Release lock, restore interrupt state.
	 */
	spin_unlock_irqrestore(&sl, flag);


	/*
	 * Type 3: Locking between two interrupt context
	 *
	 * 1. Take Lock 
	 * 2. Disable interrupt only on local cpu; unconditionally do not check
	 * 	if the interrupt is enabled or not.
	 */
	spin_lock_irq(&sl);

	/*
	 * Release lock; enable interrupt.
	 */
	spin_unlock_irq(&sl);
	
	/*
	 * Type 4: Locking between user context and bottom half
	 *
	 * If data accessed from 1)process and 2) BH(bottom half) context.
	 * 1. Take Lock 
	 * 2. Disable interrupt only on local cpu; 
	 * 3. Disallow softirqs, tasklets, Work queues running on the local cpu
	 */
	spin_lock_bh(&sl);

	/*
	 * Release lock; enable interrupt, re-enables bh running.
	 */
	spin_unlock_bh(&sl);


	return 0;
}

static void __exit stops(void) {
	printk(KERN_INFO "Goodbye!\n");
}

module_init(start);
module_exit(stops);

