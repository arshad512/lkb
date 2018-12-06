#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>		// for basic filesystem
#include <linux/proc_fs.h>	// for the proc filesystem
#include <linux/seq_file.h>	// for sequence files
#include <linux/jiffies.h>	// for jiffies
#include <linux/slab.h>
#include <linux/list.h>		// for link list

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("link list");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

void print_list(void);

struct list_head mylist;

struct foo
{
	int a;
	/*
	 * Turn struct 'foo' into list.
	 * Point of head of list (first element).
	 *
	 * Other way to say, would be that list_head is not "actually" part of
	 * struct 'foo'. It is only used used as a traversal mechanism.
	 */
	struct list_head list;
};


static int __init start(void)
{
	int i;
	struct foo *fvar[5];
	struct foo temp;

	/*
	 * To add a node.
	 * First initialize "head_node" of a list
	 *
	 * Below steps shows how its done dynamically
	 */
	INIT_LIST_HEAD(&mylist);

	/*
	 * Second... Allocate the structure which list_head is
	 * embedded. In this case struct 'foo'
	 */
	for(i=0; i<5; i++) {
		fvar[i] = kmalloc(sizeof(struct foo), GFP_KERNEL);
		if (!fvar[i]) {
			pr_info("Kmalloc Failed %d\n", i);
			return -1;
		}
		fvar[i]->a = i*100;
	}

	/* add list - this API adds at the front. Used to implement stack */
	list_add(&fvar[0]->list, &mylist);
	list_add(&fvar[1]->list, &mylist);
	list_add(&fvar[2]->list, &mylist);
	list_add(&fvar[3]->list, &mylist);
	list_add(&fvar[4]->list, &mylist);

	print_list();

	/* Add node at the end of list - 1400 added after last node */
	temp.a = 1400;
	list_add_tail(&temp.list, &mylist);

	print_list();

	/* delete a node */
	list_del(&temp.list);
	list_del(&fvar[0]->list);

	print_list();

	/* free data */
	for(i=0; i<5; i++)
		kfree(fvar[i]);

	return 0;
}

static void __exit stops(void)
{
}

void print_list()
{
	struct foo *f;

	/* list iter */
	list_for_each_entry(f, &mylist, list) {
		pr_info("Data = %d\n", f->a);
	}
	pr_info("==============\n");
}

module_init(start);
module_exit(stops);
