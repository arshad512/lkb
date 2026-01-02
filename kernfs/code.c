#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/security.h>
#include <linux/hash.h>
#include <linux/kernfs.h>
#include <linux/cgroup.h>

/* Sill WIP...
 * kernfs is a generic filesystem infrastructure within the kernel. It helps in
 * creating and managing kernel-managed namespaces in a consistent manner
 *
 * Compile tested only on: 4.18. May _not_ work on 6.X kernels.
 */

MODULE_VERSION("110");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A kernfs Linux module");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

struct fruit
{
	char name[32];
	int i;
	struct kernfs_root *kf_root; /* form an active hierarchy */
	struct kernfs_ops *kf_ops;
	atomic_t refcnt;
};

static int fruit_show_options(struct seq_file *seq,
			       struct kernfs_root *kf_root) { return 0; }

static int fruit_mkdir(struct kernfs_node *parent_kn, const char *name,
			umode_t mode) { return 0; }

static int fruit_rmdir(struct kernfs_node *kn) { return 0; }

static int fruit_rename(struct kernfs_node *kn, struct kernfs_node *new_parent,
			 const char *new_name_str) { return  0; }

static struct kernfs_syscall_ops fruit_syscall_ops = {
	.show_options		= fruit_show_options,
	.mkdir			= fruit_mkdir,
	.rmdir			= fruit_rmdir,
	.rename			= fruit_rename,
};


static int __init km_init(void)
{
	struct fruit root;
	struct kernfs_open_file *of = NULL;

	printk(KERN_INFO "Hello! %x\n", KERNFS_FILE);

	root.kf_root = NULL;
	of = NULL;
	root.kf_root = kernfs_create_root(&fruit_syscall_ops,
					   KERNFS_ROOT_CREATE_DEACTIVATED,
					   NULL);
	return 0;
}

static void __exit km_exit(void) {
	printk(KERN_INFO "Goodbye\n");
}

module_init(km_init);
module_exit(km_exit);

