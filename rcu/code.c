#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/preempt.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RCU Example");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * RCU or Read, Copy, Update is a synchronization mechanism where there are
 * multiple reader threads. And a "single" writer thread. (remember, "single",
 * Although there may be multiple writer thread, only a single writer thread
 * would be active at a given time). This can be consider as improved
 * reader-writer locking.
 *
 * What this means is that with mostly reads the performance would be very fast.
 * As there would be no race. For race to happen there must be at-least one write.
 * If there are no writes and only reads there will be no race. With write coming
 * occacionally and in-between,the only synchnonrizaiton required would be when
 * "update" or an actual write will happen.
 *
 * The engine that drives RCU is the "publishing protocol". What this says it that
 * the multiple readers could happly read a shared resouce within a rcu_lock(), and
 * when a writer is about to update a value it will stop the readers from reading the
 * shared resouce. Then the writer would update the value and enable the readers to
 * again read the upadted value.
 *
 * For example:
 *
 * There are 70 readers threads
 * There are 1 writer thread
 *
 * Now,
 * 50 reader threads are reading.
 * 20 reader threads are idle not doing anyting.
 *
 * So, All 50 reader thread would be doing the below individually... Remember, no
 * reader will borrow the pointer of other. Stale rcu_read_lock() is not valid.
 * rcu_read_lock()
 * 	// read and do work
 * rcu_read_unlock()
 *
 * During update/write of a value. A single writer thread will make
 * the shared data not accaible by new reader. In this case that 20 threads now
 * cannot access the shared data. Secondly, it will wait for the reader threads
 * which has already seen the old value. In this case it is 50 threads, which already
 * saw the old value and was processing them. That is is done by writer thread by
 * calling synchronize_rcu().
 *
 * Writer would allocate new struct and fill that up with value. There is no race yet.
 * It will then replace the root pointer (rcu_head) with the new one. The old rcu_head
 * is still there just that it is hidden. The lock is taken just for the switch, there
 * is not delete and therefore the performance is fast.
 *
 */

#define element 5

struct mylist {
	int data; // shared data.
	struct list_head node;
	/*
	 * rcu_head struct is embedded in data structure which wants
	 * to use RCU for update and lookup
	 *
	 * Each update will have a new rcu_head. The old rcu_head is hidden from
	 * readers and not deleted this gives preformance improvement.
	 */
	struct rcu_head rcu;
};

static LIST_HEAD(head_mylist);
static spinlock_t mylist_lock;
struct mylist *val[element];
struct mylist new_node;
struct mylist *save_old_pointer;
int flag;

static void destroy_old_structure(struct rcu_head *rcu_h)
{
	struct mylist *t = NULL;
	t = container_of(rcu_h, struct mylist, rcu);
	printk(KERN_INFO "Deleted old reference\n");
	if (!t) {
		printk(KERN_INFO "Value (t) not valid. will exit.\n");
		return;
	}
	/* this node should be deleted, node where data is 20 */
	printk(KERN_INFO "Data = %d\n", t->data);
	//t->data = 0;
	kfree(t);
}

static int __init start(void)
{
	int i;
	struct mylist *tmp1;

	printk(KERN_INFO "Hello!\n");

	/* add value */
	for(i=0; i<element; i++) {
		val[i] = kmalloc(sizeof(struct mylist), GFP_KERNEL);
		if(!val[i]) {
			printk(KERN_INFO "Malloc failed for %d\n", i);
			return -1;
		}
		val[i]->data = 10 * i;
	}

	/* Add a node */
	for(i=0; i<element; i++) {
		spin_lock(&mylist_lock);
		/* Add entry to rcu protected list */
		list_add_rcu(&val[i]->node, &head_mylist);
		spin_unlock(&mylist_lock);
	}

	/*
	 * READER:
	 *
	 * All readers must access shared data within
	 * rcu_read_lock() and rcu_read_unlock()
	 *
	 * Also code must not sleep within rcu read lock held.
	 */
	list_for_each_entry(tmp1, &head_mylist, node) {
		/* This is actually only disable premetion */
		rcu_read_lock();

		/* access shared data */
		printk(KERN_ERR "value(0) = %d\n", tmp1->data);

		/* enable premeption */
		rcu_read_unlock();
	}

	/*
	 *  Wait for all readers to finish. This ensures that there
	 *  is no reader in criticle section and older refrence (rcu_head)
	 *  could be deleted.
	 */
	synchronize_rcu();

	/*
	 * WRITER (Update)
	 *
	 * 01. Allocate new structure and assign new value. (new_node.data = 1400)
	 * 02. Replace old struct with new structure (list_replace_rcu)
	 */
	list_for_each_entry(tmp1, &head_mylist, node) {

		rcu_read_lock();
		/*
		 * I know for sure there is value of 10. i * 10
		 * where i == 1;
		 */
		if (tmp1->data == 10) {
			
			/* update this value to 1400 */
			new_node.data = 1400;

			spin_lock(&mylist_lock);
			/* replace old entry with new entry */
			list_replace_rcu(&tmp1->node, &new_node.node);
			spin_unlock(&mylist_lock);
		}
		rcu_read_unlock();
	
	}
	
	/*
	 *  Wait for all readers to finish.
	 */
	synchronize_rcu();


	/*
	 * READER again:
	 */
	list_for_each_entry(tmp1,&head_mylist, node) {
		rcu_read_lock();

		/* access shared data */
		printk(KERN_ERR "value(1) = %d\n", tmp1->data);

		rcu_read_unlock();
	}

	/*
	 * WRITER (delete)
	 *
	 * 01. Delete node with value == 20
	 * 02. Del struct (list_replace_rcu)
	 */
	flag=0;
	spin_lock(&mylist_lock);
	list_for_each_entry(tmp1, &head_mylist, node) {

		//rcu_read_lock();
		/*
		 * I know for sure there is value of 20. i * 10
		 * where i == 2;
		 */
		if (tmp1->data == 20) {
			// Delete node from RCU protected list
			flag = 1;
			list_del_rcu(&tmp1->node);
			spin_unlock(&mylist_lock);
			/*
			 * Dispose/destroy the node. As it is confirmed that the
			 * there is no reader pending
			 */
			call_rcu(&tmp1->rcu, destroy_old_structure);
			
		}
		//rcu_read_unlock();
	}

	if (flag == 0)
		spin_unlock(&mylist_lock);
	
	
	/*
	 * READER again,again:
	 */
	list_for_each_entry(tmp1,&head_mylist, node) {
		rcu_read_lock();

		/* access shared data */
		printk(KERN_ERR "value(2) = %d\n", tmp1->data);

		rcu_read_unlock();
	}

	return 0;
}

static void __exit end(void)
{
	printk(KERN_INFO "Goodbye\n");
}

module_init(start);
module_exit(end);
