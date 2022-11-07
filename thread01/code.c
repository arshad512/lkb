#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>		// for link list
#include <linux/kthread.h>	// kernel thread
#include <linux/err.h>		// IS_ERR
#include <linux/slab.h>
#include <linux/delay.h>	// mdelay etc...

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux thread module example");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

#define ELEMENTS 5
#define SLEEP 1000 /* 1 seconds */

int mfoo(void *arg);
void start_mythread(void);

struct list_head gwl; /* global_work_list */
struct mywork *g_work[ELEMENTS];

struct mywork
{
	struct task_struct *task;
	struct list_head list;
};


/*
 * purpose of this exercise is to create thread. Store it under a list
 * and later execute it.
 */
static int __init km_init(void)
{
	char task_name[32];
	int i;

	/* allocate mywork struct */
	for(i=0; i<ELEMENTS; i++) {
		g_work[i] = kmalloc(sizeof(struct mywork), GFP_KERNEL);
		if (!g_work[i]) {
			pr_info("Kmalloc Failed %d\n", i);
			return -1;
		}

		/* Create a kernel thread. Thread created will be in
		 * stopped state */
		memset(task_name, 0, 32);
		sprintf(task_name, "my_super_task%d",i);

		g_work[i]->task = kthread_create(mfoo, NULL, task_name);
		if (IS_ERR(g_work[i]->task)) {
			printk(KERN_INFO "kthread failed for:%d\n", i);
			return -1;
		}
	}

	pr_info("All task created!\n");
	INIT_LIST_HEAD(&gwl);

	/* add list - this API adds at the front. Task still at stopped 
	 * state.
	 */
	for(i=0; i<ELEMENTS; i++)
		list_add(&g_work[i]->list, &gwl);

	start_mythread();

	return 0;
}

static void __exit km_exit(void) {
	struct mywork *work;
	int ret, i = 0;

	/* loop and stop thread */
	list_for_each_entry(work, &gwl, list) {
		msleep(2000);
		/* stop thread. This is blocking */
		ret = kthread_stop(work->task);
		if(!ret)
			printk(KERN_INFO "Thread stopped:%d",i++);
	}

	/* free data */
	for(i=0; i<ELEMENTS; i++)
		kfree(g_work[i]);
	printk(KERN_INFO "Goodbye\n");
}

int mfoo(void *arg) {
	/* when kthread_stop() is called. kthread_should_stop()
	 * returns true. Else it returns false. This means until
	 * kthread_stop is called the while loop is valid.
	 *
	 * In this example. It might be possible that the loop
	 * would finish early than kthread_stop is called. This
	 * will call the loop over and over again until
	 * "rmmod code" is issued which will is calling kthread_stop()
	 * and kthread_stop() will force kthread_should_stop() to
	 * return 'true' breaking the loop.
	 */
	while (!kthread_should_stop()) {
		pr_info("My thread is running...\n");
		msleep(SLEEP);
	}

	return 0;
}

void start_mythread(void)
{
	struct mywork *work;
	int i =0;

	/* loop under list and make thread runnable. this will make
	 * mfoo() call 5 times
	 */
	list_for_each_entry(work, &gwl, list) {
		pr_info("Data = %d\n", i++);
		wake_up_process(work->task);
	}
	pr_info("==============\n");
}

module_init(km_init);
module_exit(km_exit);
