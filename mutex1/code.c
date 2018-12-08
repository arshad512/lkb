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
MODULE_DESCRIPTION("mutex");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * Mutex: Synchronization mechanism.
 *
 * Statically
 * DEFINE_MUTEX(m)
 *
 * Dynamically
 * struct mutex m;
 * mutex_init(&m);
 * 
 * Single task only can hold mutex.
 * Multiple unlocks are _not_ permitted.
 * Task holding mutex must not block,exit,get killed or else
 * 	mutex will remained locked forever.
 * Operation on mutex while it is held is not permitted.
 * 	Eg, re-initialization of mutex while it is held.
 * Mutex cannot be used in atomic context. As they can sleep.
 *
 */

static int __init start(void)
{
	struct mutex m;
	int i;

	printk(KERN_INFO "Start...!\n");
	/*
	 * Initialize Mutex
	 */
	mutex_init(&m);
	/*
	 * Lock mutex. Take a lock.
	 */
	mutex_lock(&m);

	/*
	 * check whether a mutex is locked or unlocked - just check.
	 * 1 = locked
	 * 0 = unlocked
	 */
	i = mutex_is_locked(&m);
	if (i == 1 )
		printk(KERN_INFO "Mutex is locked\n");
	else
		printk(KERN_INFO "Mutex is NOT locked\n");

	/*
	 * if mutex not lock, take a lock and return 1
	 * else return 0 if mutex is locked.
	 */
	i = mutex_trylock(&m);
	if (i == 1 )
		printk(KERN_INFO "mutex_trylock - Mutex was free taken lock\n");
	else
		printk(KERN_INFO "mutex_trylock - Mutex already locked, return zero\n");

	/* Do your work */


	/* Unlock mutex - Release lock */
	mutex_unlock(&m);

	/*
	 * mutex_lock() is not interruptible.
	 * if driver needs to be interrupted by signal it must
	 * use mutex_lock_interruptible() - This can be interrupted
	 * by any signal.
	 *
	 * Return 0 is lock was taken.
	 * Reutrn -EINTR if lock taking is interrupt by signal. 
	 * This must be handled by the user.
	 *
	 */
	i = mutex_lock_interruptible(&m);
	if (i == 0)
		printk(KERN_INFO "mutex_lock_interruptible - Lock taken.\n");
	else
		printk(KERN_INFO "mutex_lock_interruptible - Interruptible by signal\n");

	/* Do your work */
	
	/* Unlock mutex - Release lock */
	mutex_unlock(&m);
	 
	/*
	 * Finally, if driver choose to be only interrupted by kill signal
	 * and no other signal then it must use function
	 * mutex_lock_killable()
	 */
	i = mutex_lock_killable(&m);
	if (i == 0)
		printk(KERN_INFO "mutex_lock_killable - Lock taken.\n");
	else
		printk(KERN_INFO "mutex_lock_killable - Interrupted by kill signal\n");
	
	/* Do your work */
	
	/* Unlock mutex - Release lock */
	mutex_unlock(&m);

	return 0;
}

static void __exit stops(void) {
	printk(KERN_INFO "Goodbye!\n");
}

module_init(start);
module_exit(stops);

