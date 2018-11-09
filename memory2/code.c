#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>  /* error codes */
#include <linux/sched.h>
#include <linux/mm.h> /* find_vma */
#include <linux/mm_types.h> /* vm_area_struct */
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h> /* kmap/kunmap */

MODULE_LICENSE("GPL");
MODULE_VERSION("110");
MODULE_DESCRIPTION("memory fucntion examples");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

/*
 * kmalloc relies on slab allocator for allocations.
 * this helps in elimination of fragmentation in memory.
 * speed up memory allocation of commonly used objects.
 *
 * slab = continuous memory made up of page frames. 
 * 	- can be empty
 * 	- can be full
 * 	- can be partially empty or full
 * cache = made up of one or more slab
 * REMEMBER!!! - It is up to memory allocator to build up cache.
 *
 * when code allocates memory:
 * 	- it looks for matching size slab.
 * 	- if not found allocate new slab and add to cache.
 * 	- if found allocate from slab
 *
 * slab vs slob vs slub = these are different type of allocator in kernel.
 * slab = continuous memory, design for speed. (Cache friendly)
 * slob = - same - but design for space. Very compact and small footprint
 * slub = - same - but fastest (very very fast)
 *
 */

static int __init memory_init(void)
{
	struct page *p;
	int *ptr;
	//unsigned long va; /* virtual address returned */
	unsigned long phy;
	
	/* zero allocate */
	ptr = kzalloc(sizeof(int)*5,GFP_KERNEL);
	if(!ptr) {
		printk("kzalloc failed\n");
		return -1;
	} else {

		/* free */
		kzfree(ptr);
	}

	/* 
	 * allocate.
	 * Returns a memory area in the kernel permanent mapping
	 * which means physical contiguous. 
	 */
	ptr = kmalloc(sizeof(int)*5,GFP_KERNEL);
	if(!ptr) {
		printk("kmalloc failed\n");
		return -1;
	} else {
		/*
		 * can convert to physical memory since
		 * this is continuous
		 */
		phy = virt_to_phys(ptr);

		/* free */
		kfree(ptr);
	}

	/*
	 * vmalloc - allocate memory from HIGH_MEM
	 * Only continuous on virtual memory not physical.
	 * HIGH_MEM : Address cannot be translated to physical or bus address
	 * cannot be used for DMA etc...
	 *
	 * vmalloc() slower than kmalloc, because...
	 * 	- retrieve memory
	 * 	- build page tables, then returns.
	 *
	 * /proc/vmallocinfo - 
	 */
	ptr = vmalloc(40960);
	if(!ptr) {
		printk("vmalloc failed\n");
		return -1;
	} else {
		/* free */
		vfree(ptr);
	}

	/*
	 * kmap()/kunmap()
	 *
	 * kernel permanent  maps 896MB to lower memory
	 * this is 1:1 mapping
	 *
	 * When it comes to memory higher than 896MB. 
	 * We have to specially map memory using kmap()
	 * Maps page/pages to lower range from higher range
	 */
	p = alloc_pages(GFP_HIGHUSER, 3); /* allocate high mem */
	if(!p) {
		printk(KERN_INFO "alloc_pages Allocation failed\n");
		return -1;
	} else {
		printk(KERN_INFO "alloc_pages Allocation done\n");

		/* Map struct 'p' to lower ptr */
		ptr = (unsigned int *)kmap(p);

		/* unmap */
		kunmap(p);

		/* free */
		__free_pages(p,3);
	}

	return 0;
}

static void __exit memory_exit(void) {
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(memory_init);
module_exit(memory_exit);

