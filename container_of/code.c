#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>		// for basic filesystem
#include <linux/proc_fs.h>	// for the proc filesystem
#include <linux/seq_file.h>	// for sequence files
#include <linux/jiffies.h>	// for jiffies
#include <linux/slab.h>

#define YES 1

/*
 * container_of macro give the pointer of the whole structure
 * which wraps the member, by just having a pointer to a member.
 *
 * For below example, if only have pointer to member 'furry' or 'name'
 * using cointainer_of macro we can retrive value of struct kitten.
 */

struct kitten
{
	int furry; /* of course yes :-) */
	char *name;
};

static int __init start(void)
{
	struct kitten tom;
	struct kitten *new_kitten;
	char *pname;

	tom.furry = YES;

	tom.name = kmalloc(25, GFP_KERNEL);
	strcpy(tom.name, "tom_cat");

	pr_info("name = %s\n", tom.name);
	pname = tom.name;
	pr_info("name2 = %s\n", pname);

	/* pointer of type = cointainer_of(pointer, struct <type>, member name) */
	new_kitten = container_of(&pname, struct kitten, name);
	pr_info("Name3 = %s\n", new_kitten->name);
	pr_info("Furry0 = %d\n", tom.furry);
	/* Not all member value is copied, below will be garbage */
	pr_info("Furry = %d\n", new_kitten->furry);
	/* reassign new value , to get valid value*/
	new_kitten->furry = 100;
	pr_info("Furry1 = %d\n", new_kitten->furry);
	return 0;
}

static void __exit stops(void)
{
}

module_init(start);
module_exit(stops);

MODULE_LICENSE("GPL");

