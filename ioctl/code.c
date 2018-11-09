#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h> /* support kernel driver model */
#include <linux/kernel.h>
#include <linux/errno.h>  /* error codes */
#include <linux/sched.h>
#include <linux/mm.h> /* find_vma */
#include <linux/mm_types.h> /* vm_area_struct */
#include <linux/slab.h>
#include <linux/fs.h> /* file system specifics */
#include <linux/uaccess.h> /* Require for copy to user function */
#include <linux/vmalloc.h>
#include <linux/highmem.h> /* kmap/kunmap */
#include <linux/types.h> /* dev_t * defs */
#include <linux/version.h> /* KERNEL_VERSION */

/* SCSI */
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_devinfo.h>
#include <scsi/sg.h>

MODULE_LICENSE("GPL");
MODULE_VERSION("110");
MODULE_DESCRIPTION("Linux module,ioctl example");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

#define DEVICE_NAME "mmapchar" 
#define QUERY_GET_VARIABLES _IOR('q', 1, int *)
#define QUERY_CLR_VARIABLES _IO('q', 2)
#define QUERY_SET_VARIABLES _IOW('q', 3, int *)


/*
 * ioctl: input/output control.
 *
 * Send special commands to devices. Which are outside the
 * system call (Linked to file operations)
 *
 * Examples are: reset, shutdown... if the ioctl is not defined
 * in device driver the driver returns ENOTTY.
 *
 * Note!!! ioctl must be unique accross system.
 *
 * Macros to help create ioctl's
 * 01.	_IO(MAGIC, SEQ_NO) - Does not need data transfer
 * 02.	_IOW(MAGIC, SEQ_NO, type) - Write (copy_from_user or get_user)
 * 03.	_IOR(MAGIC, SEQ_NO, type) - Read (copy_to_user or put_user)
 * 04.	_IORW(MAGIC, SEQ_NO, type) - Both read/write
 *
 * MAGIC = positive integer 0 - 255
 * SEQ_NO = 8 bit command id
 * type = data type to be read or written.
 *
 * Example:
 * #define RESET_DEVICE _IOR('q', 1, int *)
 *
 * The above macro uses the 'q' as the MAGIC and 1 as sequence number.
 * The transfer type is of int * (4 bytes) in 32bit system. If more
 * complex type is requried then a struct can be passed.
 *
 * Read ioctl.c to see how it is used.
 *
 * NOTE: since ioctl is unique, header files created for kernel should be
 * replicated in user space.
 *
 */

int majorNumber = 150;

/*
 * The prototype functions for the character driver
 */
int dev_open(struct inode *, struct file *);
int dev_release(struct inode *, struct file *);
//static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
//static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
long dev_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations fops =
{
	.open = dev_open,
	//.read = dev_read,
	//.write = dev_write,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
	/*
	 * older version takes BKL. Makes system slow.
	 * BKL = Big kernel lock.
	 */
	.ioctl = dev_ioctl,
#else
	/*
	 * Newer kernals has unlocked_ioctl()
	 * Older kernels used to have ioctl()
	 *
	 * unlocked_ioctl allows driver writers to
	 * choose what lock to take instead of standard BKL
	 */
	.unlocked_ioctl = dev_ioctl,

	/*
	 * compat_ioctl allows 32bit user space to
	 * make calls to 64bit kernel
	 */
	//.compat_ioctl = dev_ioctl,
#endif
	.release = dev_release,
};

static int __init ioctl_init(void)
{
	int ret;

	/* mknod /dev/mmapchar c 150 0 */
	ret = register_chrdev(majorNumber, DEVICE_NAME, &fops);
	if (ret<0) {
		printk(KERN_ALERT "failed to register a major number %d\n",ret);
		return ret;
	}
	printk(KERN_INFO "registered with major number %d\n",
			majorNumber);

	return 0;
}

/* open */
int dev_open(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "Device opened\n");
	return 0;
}

/* Release */
int dev_release(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "Device successfully closed\n");
	return 0;
}

long dev_ioctl(struct file *filep,
		unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	unsigned long a=14;
	unsigned long x;

	switch (cmd)
	{
		case QUERY_GET_VARIABLES:
			/* return value from kernel to user: read */
			if(copy_to_user((int *)arg, &a, sizeof(unsigned long))) {
				printk(KERN_INFO "COPY_TO_USER FAILED\n");
				rc =  -EACCES;
			}
			break;
		case QUERY_CLR_VARIABLES:
			printk(KERN_INFO "CLR_VARIABLES .....\n");
			break;
		case QUERY_SET_VARIABLES:
			/* take value from user to kernel: write */
			if(copy_from_user(&x, (unsigned int *)arg, sizeof(unsigned long))) {
				printk(KERN_INFO "COPY_FROM_USER FAILED\n");
				rc =  -EACCES;
			} else {
				printk(KERN_INFO "I got [%lu] from user\n", x);
			}
			break;
		default:
			printk(KERN_INFO "No matching IOCTL\n");
	}

	//printk(KERN_INFO "ioctl now %u, %lu\n",cmd, arg);
	return rc;
}

static void __exit ioctl_exit(void) {
	unregister_chrdev(majorNumber, DEVICE_NAME);
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(ioctl_init);
module_exit(ioctl_exit);

