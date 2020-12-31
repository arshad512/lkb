#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A noop Linux module. With debug enabled in Makefile");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * Adding EXTRA_CFLAGS results '-ggdb' getting adding to the
 * build command line. This could be verified by checking the
 * .code.o.cmd file.
 */

static int __init km_init(void)
{
	printk(KERN_INFO "Hello!\n");
	return 0;
}

static void __exit km_exit(void) {
	printk(KERN_INFO "Goodbye\n");
}

module_init(km_init);
module_exit(km_exit);
