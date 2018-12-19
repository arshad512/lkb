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
#include <linux/rbtree.h> /* rbtree */
#include <linux/radix-tree.h> /* radix tree */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Radix tree");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * Radix tree.
 *
 * Radix tree is based on Tire tree. Data structure used to 
 * construct Associative array or maps. These are used for 
 * searching strings and is faster than binary search trees
 * as comparison of each small string is very small at each level.
 *
 * Properties are :
 * -	Nodes store associative key (should Prefer String)
 * -	Values are stored in leaves and inner nodes.
 * - 	Decedents all have common prefix. Something like below.
 *
 *   			[c,d]
 *   			/  \
 *   	              [a]  [o]
 *                    / \    \
 *                  [t] [r]  [g]
 *
 *       Search for "do", "dog", "car" and "cat"(favourite :))
 *
 * Below code example shows the use of Radix tree API in kernel.
 * There is nothing useful in the example, just showing the
 * use of calling the API. Also notice the key used is 'int'
 * not ideal but for this example it was chosen to demonstrate
 * the API use.
 */

struct radix_tree_root mytree;


static int __init start(void)
{
	int ret;
	unsigned long key,failkey;
	void *v,*r;
	int item;

	printk(KERN_INFO "Start...!\n");
	
	/* Initialize root of radix tree at runtime */
	INIT_RADIX_TREE(&mytree, GFP_KERNEL);

	/* hard code key */
	key = 5;
	/* hard-code 'failkey' - The search on this will fail */
	failkey = 50;
	/*
	 * item or value :- key ->[points to]-> value
	 * key(5) -> value(14)
	 */
	item = 14;

	/* populate void pointer */
	v = &item;

	/* 
	 * Insert into radix tree.With key and value (v)
	 * Return 0 on failure
	 */
	ret = radix_tree_insert(&mytree,key,v);
	if (ret != 0) {
		printk(KERN_INFO "insert failed\n");
		return -1;
	}
	printk(KERN_INFO "insert success\n");
	
	
	/* 
	 * Once value is inserted in radix tree.
	 * lookup for its presence.
	 * 
	 * This lookup will fail as we are forcing a
	 * search on wrong key.
	 */
	r = radix_tree_lookup(&mytree, failkey);
	if (!r) {
		printk(KERN_INFO "lookup failed\n");
	} else {
		printk(KERN_INFO "lookup success\n");
	}

	/*
	 * This search will succeed
	 * On failure Return NULL
	 * !NULL on success
	 */
	r = radix_tree_lookup(&mytree, key);
	if (!r) {
		printk(KERN_INFO "lookup failed\n");
	} else {
		/* Also print the value, when search is successful */
		printk(KERN_INFO "lookup success %d\n", *(int*)r);
	}

	/*
	 * Delete fill fail as we are force providing 
	 * wrong key.
	 */
	r = radix_tree_delete(&mytree, failkey);
	if (!r) {
		printk(KERN_INFO "delete failed\n");
	} else {
		printk(KERN_INFO "delete success %d\n", *(int*)r);
	}
	
	/*
	 * Delete a key from radix tree
	 * On failure Return NULL
	 * !NULL on success
	 */
	r = radix_tree_delete(&mytree, key);
	if (!r) {
		printk(KERN_INFO "delete failed\n");
	} else {
		/* Also print the value, when delete is successful */
		printk(KERN_INFO "delete success %d\n", *(int*)r);
	}

	return 0;
}

static void __exit stops(void) {
	printk(KERN_INFO "Goodbye!\n");
}

module_init(start);
module_exit(stops);

