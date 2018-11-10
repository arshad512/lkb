#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A noop Linux module");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");
	

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
