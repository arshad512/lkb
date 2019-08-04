# lkb
## The linux kernel programming guide.
## Arshad Hussain
### arshad.super@gmail.com


### Important Data Structures

#### Process
```
struct task_struct : Represent a process within a kernel
struct file_struct: Keeps info of open file
struct fs_struct: Maintains association of process with struct FILE
```
#### Filesystem
```
struct super_block: Keeps information of all mounted file system. 
struct superblocks: Keeps information of whole filesysem
struct inode: Represent file on disk
struct file: Represent an open file.
struct dentry: Keeps information of directory structure.
struct vfsmount: Keeps mount information.
struct nameidata: Keeps path information.
```
#### Memory Management
```
struct mm_struct: Summary of whole process memory.
struct vm_area_struct: Maintains individual summary of segments within a process. Eg code, data,heap and mmap 
struct namespace: Data caching associated with inode.
struct page: Represent a portion of physical memory
```
#### Block Layer
```
struct request_queue: IO queue associated with device
struct request: Individual request within a request_queue
struct bio: Represents an unit of IO. A request_queue have many bio’s
struct bio_vec: Keeps 1) buffer info which is to be transferred, 2) start end of the buffer to be transferred. 3) struct page information 
struct gendisk: Represent a physical disk.
```
### Memory Management

During boot, the physical RAM is determine by the BIOS and passed to Linux Kernel. The address space is determine by the segment selector which points to a flat 32bit address. This is compulsory and every address must pass through this. This is called **LINEAR ADDRESS**. This liner address is then passed through paging unit and this is what we use. On a protected mode OS, physical memory cannot access without translation. All user-space and kernel-space gets a virtual address which must be translated (using MMU) to ultimately reach the physical RAM. To speed up translation, a cache of already translated VA to PA is kept and is know as TLB (Translated look aside buffer). If mapping is found in TLB the physical address is returned without going through translation.
```
Virtual Memory 
     |___TLB
     |___MMU<———->[Physical RAM]
```

When Linux boots it normally loads itself at an offset of 16MB from start of RAM. It leaves the first 16MB for DMA and stuff. This is the linux kernel physical address. It then maps itself at an offset of **“PAGE_OFFSET”** which is 3GB on 32bit systems. This is the start of the linux kernel virtual address. From PAGE_OFFSET to 896MB there is a persistent mapping between kernel virtual address and physical address. This region is also called **LOW MEMORY**. Anything beyond this has to be mapped on the fly (using kmap and kummap calls) and is known as **HIGH MEMORY**. 

We can see that PA = VA - PAGE_OFFSET.  Therefore, if kernel is accessing a kernel virtual address of 3GB+1. It is in reality actually accessing the PA of 1.

```
CPU
  |____[SEGMENT] ———-------------------> [GDT] 
                                          |——> [ 4GB Liner Address space (Physical RAM Flat) ]
                                          |    
                                [Page Table]
                                          |
                                          \/
           [XXXXXXXX PHYSICAL RAM (PAGED) XXXXXXXX] -B
                                          |
                                         \/
    [——This is MAPPED to virtual Address  (LOGICAL ADDRESS)—-]  
[XXXXXXXXXXXXXXX VIRTUAL ADDRESS XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX] -A
       ^                                             ^
      16MB                                          3GB
      Kernel Physical                           (PAGE_OFFSET)
      Address                                   Kernel virtual Address


```

This brings us to important concepts. 
1. Physical Address: This is address which points directly to RAM. See Diagram -B
2. Virtual Address: This is address which only makes sense when there is a successful translation to point to real RAM. This is sort of a fake address which MUST point to (map) to a real backing RAM. See Diagram -A
3. Logical Address: This is virtual address which has a mapping. Mostly this is PAGE_OFFSET + 896MB. Anything beyond that needs to be mapped as mentioned earlier. From the above diagram, the 3GB+1 byte has a 1:1 mapping to 16MB+1 byte. Which in turn goes through page tables/MMU to yield real physical address.
4. Linear Address: This is address which is yet to go through paging. This will ultimately form the logical address. Liner address is accessed using CS:Offset

#### API to work with Memory

kmalloc(): This API returns a logical address. Returns address from LOW_MEMORY. That is, it is virtual and mapped. Since it is pre-mapped this is continuos in virtual as well as physical address space. That is, it is continuous in diagram -A as well as diagram -B. Memory which is continuous in physical memory can be converted to and from virtual to physical. This API returns number of bytes requested and depending on the flags this API sleeps.  GFP_KERNEL flag can sleep. GFP_ATOMIC for interrupt and will not sleep.

vmalloc(): This API ultimately returns a logical address. It first gets memory from HIGH_MEMORY then creates a map and converts it into logical address and then then returns to user. This makes vmalloc() slower as it fetches memory and creates the mapping. This also results in vmalloc() returning memory which is not continuous in physical memory, however it is continuous in virtual memory.

alloc_page(): This API allocates a single page. This is physical memory.

alloc_pages(): This API allocates multiple pages. This is physical memory.

page_address(): This API returns virtual address that corresponds to the start of the page. This API is used to convert physical address to virtual address. VA = page_address(alloc_page()); This is the general use of this API

virt_to_page(): This API is used to convert virtual address to page. struct page *p = virt_to_page(unsigned int *virtual_address);

__get_free_pages(): This API is used to get pages but return page/s is in virtual address.

kmap(): Create mapping of pages to virtual address.

kumap(): Remove mapping


#### Allocators

Linux kernel gives mechanisms to create pools for specific objects. When there is a need to add and remove object frequently these pools could be created. This speeds things up and avoids or reduces fragmentations. 

API to create such pools are :-

1. kmem_cache_create(): Create a pool.
2. kmem_cache_alloc(): Allocate an object from this pool.
3. kmem_cache_free(): Return object back to pool
4. kmem_cache_destroy(): Destroy 

#### Different types of Allocators that can be tuned in Linux Kernel

1. Slab = Continuous memory, design for speed. (Cache friendly)
2. Slob = - Same - but design for space. Very compact and small footprint
3. Slub = - Same - but fastest (very very fast)

#### mmap
mmap() call is part of file_system operation which maps kernel memory to user space. In other words mmap remaps kernel memory region into user-space address space. Internally, mmap calls **remap_pfn_range()** from within kernel to map physical memory to user-space process.

#### User space to Kernel space transfer

User space stack pointer is discouraged and not used in kernel space. Therefore if data needs to be transfer between  user-space and kernel space specific API must be used.

- copy_to_user(): Copy from kernel to user space.
- copy_from_user(): Copy from user space to kernel space

#### Virtual Memory Area (VMA)

Linux kernel keeps track of process mapping using VMA. Individual Process VMA includes 
- code area
- heap area
- data area
- stack area.

VMA is part of mm_struct and is organised as below.
```
[Current] ———> [task_struct] -----> [ mm_struct ]
                                         |—————> [vm_area_struct] ——> TEXT
		                         |
                                         |------>[vm_area_struct] ——-> DATA
                                         |
                                         |------->[vm_area_struct] ——-> MMAP
					 |
				         |-----> ETC
```

### FILESYSTEM

Filesystem or file-system is the part of kernel code that allows users to create, write and retrieve data or files in a very easy manner. How the disk space is utilised, where file is stored this is hidden from user. Linux goes one step ahead, by allowing a plethora of filesystem from which users can select for his purpose. This is achieved by having an abstraction layer called VFS which sits on top of file system. Let see how this works.

#### The VFS

VFS or the Virtual Filesystem Switch is an abstraction layer. This layer has set of operations which it supports. For few operations this layer defines them which we call as generic operations and is not FS specific. For other it lets the filesystem define them. For example the allocation of a block for ext2 will be different from fat32 or a XFS. This is defined by their respective FS. Every FS which is written for Linux kernel must support these minimum VFS operations. 

- Super block operations : How whole FS is defined and  is mounted and unmounted
- Inode operation : Where files are stored and how its buffer is created
- Dentry operation: How traverse is done
- File operation: How open file is handled

```

                                    User Space Issues a Read/Write (IO)
                                    -----------------------------------
				    System Call (read()/write())
				    -----------------------------------
				    VFS intercepts the Call
 			            -----------------------------------
				    Depending upon what FS it is meant for XFS or EXT
	             		    -----------------------------------
	                            Generic Block Layer
		            	    -----------------------------------
				    Device Driver for Disk/Floppy/CDROM
			            -----------------------------------
				    Reads / Writes to Actual Device
				    and raise an Hardware Interrupt when done
```

Most of the call stack looks fairly straight forward. We will touch upon two important aspects first, where it decides what FS to make call and second how it pushes IO down to device or disk. The second will be covered more in block layer section. In this section we will discuss when any how  VFS knows about what FS to call. We take example of read/write as we go in detail.

#### IOCTL
IOCTL is a system call which is device specific and cannot be expressed with reqular system calls. Example Eject CD-ROM or Sending Specific SCSI to underlying device

#### The write path

#### The read path






