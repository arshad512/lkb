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

/* SCSI */
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_devinfo.h>
#include <scsi/sg.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mmap() Linux kernel module example");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

#define  DEVICE_NAME "mmapchar"

/*
 * mmap: map kernel memory to user space
 * mmap remaps kernel memory region into user-sapce address space.
 *
 * remap_pfn_range(): maps physical memory to user-space process.
 * this is how mmap() is implemented.
 *
 * Call mmap() on a file
 * 	1. device driver will call file_operations->mmap()
 * 	2. will inturn call remap_pfn_range()
 *
 * I/O memory is mapped differently, this is done through
 * io_remap_pfn_range()
 *
 * NOTE!!!: ioremap() is _ONLY_ used when mapping I/O memory
 * to kernel space.  To map I/O memory to user-space used
 * io_remap_pfn_range()
 *
 * system call mmap() -> is called via file_oprations->mmap()
 * Reason is, user process cannot access /dev/X device memory
 * directy. Therefore, user process calls mmap() to remap this
 * memory into its virtual address space.
 *
 * user-space
 * 	mmap(void *addr, len, prot, flag, fd, offset);
 * 	addr: User-space address where mapping will START.
 * 	len: Length of mapping
 * 	prot: Permission (protection)
 * 	flag: type of mapping, 1)private or 2) shared
 * 	fd: discriptor of file to mmap
 * 	offset: Start at offset of a file (fd). And will be
 * 		size len.
 * kernel-space
 * 	mmap(struct file *filep, vm_area_struct *vma);
 * 	filep: pointer to open device file
 * 	vma: Pointer to user-space vma where mapping should go.
 *
 *
 * Finally, unmap() to release mapping.
 *
 */

int majorNumber = 150;
char *buffer = NULL;
int SIZE = 4096;


// The prototype functions for the character driver
int dev_open(struct inode *, struct file *);
int dev_release(struct inode *, struct file *);
//static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
//static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
int dev_mmap(struct file *, struct vm_area_struct *);
//int dev_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

static struct file_operations fops =
{
	.open = dev_open,
	//.read = dev_read,
	//.write = dev_write,
	.mmap = dev_mmap,
	//.ioctl = dev_ioctl,
	.release = dev_release,
};

static int __init lkm_init(void)
{
	int ret;

	/* mknod /dev/mmapchar c 150 0 */
	ret = register_chrdev(majorNumber, DEVICE_NAME, &fops);
	if (ret<0) {
		printk(KERN_ALERT "failed to register a major number %d\n",ret);
		return ret;
	}
	printk(KERN_INFO "registered correctly with major number %d\n",
			majorNumber);

	/*
	 * This kernel mamory will be re-mapped so that
	 * the user-space can access it.
	 */
	buffer = kmalloc(SIZE, GFP_KERNEL);
	if (!buffer) {
		printk(KERN_ALERT "failed to allocate memory \n");
		return -1;
	}
	return 0;
}

/* open */
int dev_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "Device opened\n");
   return 0;
}

/* Release */
int dev_release(struct inode *inodep, struct file *filep){
	if(buffer)
		kfree(buffer);
	printk(KERN_INFO "Device successfully closed\n");
	return 0;
}

int dev_mmap(struct file *filep, struct vm_area_struct *vma)
{
	size_t size;
	/* Offset value which is passed in mmap() */
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long pfn;
	int rc = 0;

	/* len which is passed to mmap() */
	size = vma->vm_end - vma->vm_start;
	printk(KERN_INFO "Nothing for mmap now %lu %lu\n", size, offset);
	/*
	 * Requires more error checking
	 */
	pfn = virt_to_phys(buffer + offset) >> PAGE_SHIFT;

	if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
   		printk(KERN_INFO "mmap allocation FAILED \n");
		rc = -EAGAIN; // Or EIO should be returned.
	} else {
   		printk(KERN_INFO "mmap allocation PASSED \n");
		rc = 0;
	}
		
	return rc;
}

static void __exit lkm_exit(void) {
	if(buffer)
		kfree(buffer);
	unregister_chrdev(majorNumber, DEVICE_NAME);
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(lkm_init);
module_exit(lkm_exit);

