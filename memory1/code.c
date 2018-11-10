#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>  /* error codes */
#include <linux/sched.h>
#include <linux/mm.h> /* find_vma */
#include <linux/mm_types.h> /* vm_area_struct */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Memory");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * virtual address do not point directly to RAM.
 * They go through a translation (done via MMU)
 * and via page tables to get the actual physical
 * address.
 *
 * logical address are address which are directly
 * mapped to kernel space. logical address are 
 * virtual address. These are first 896MB.
 * 
 * physical address = logical address - PAGE_OFFSET 
 *
 * low memory are memory for which logical address
 * is present. Or in other words mapping are alread
 * done during boot time.
 *
 * high memory are memory which mapping are not
 * present but need to be created on the fly.
 * See other section memory2 for more details
 *
 * [virtual address/logical address]
 * 	|
 * 	\/
 * [Page tables]
 * 	|
 * 	\/
 * [Physical address]
 *
 * page is smallest unit physical memory that is accessed
 * by kernel and is kept in 'struct page'
 *
 */

static int __init code_init(void)
{
	struct page *p,*p1,*p2;
	unsigned long va; /* virtual address returned */
	unsigned long *page;

	/*
	 * Allocate single page. (Physical memory)
	 * Similar to alloc_pages(mask,0);
	 */
	p = alloc_page(GFP_KERNEL);
	if(!p) {
		printk(KERN_INFO "alloc_page Alocation failed\n");
		return -1;
	} else {
		printk(KERN_INFO "alloc_page Alloation done\n");
		__free_pages(p,0);
	}

	/* 
	 * 2^3 = 8 pages allocation will be done here
	 * Allocate set of pages and return struct page
	 */
	p1 = alloc_pages(GFP_KERNEL, 3);
	if(!p1) {
		printk(KERN_INFO "alloc_pages Alocation failed\n");
		return -1;
	} else {
		printk(KERN_INFO "alloc_pages Alloation done\n");
		/*
		 * macro page_address() - return virtual address that 
		 * corresponds to the start of page
		 */
		page = (unsigned long *)page_address(p1);

		/*
		 * convert virtual address back to page
		 */
		p2 = virt_to_page(page);
		__free_pages(p1,3);
	}

	/*
	 * Return virtual address.
	 * GFP_HIGHMEM, not to be used with this __get_free_pages
	 * As it is not gurantee to be continous.
	 */
	va = __get_free_pages(GFP_KERNEL, 3);
	if (!va) { 
		printk(KERN_INFO "__get_free_pages Alocation failed\n");
		return -1;
	} else {
		printk(KERN_INFO "__get_free_pages Alloation done\n");
		free_pages(va,3);
	}

		
	return 0;
}

static void __exit code_exit(void) {
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(code_init);
module_exit(code_exit);

