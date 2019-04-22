# lkb
```
The linux kernel programming guide.
arshad.super@gmail.com
```

### Important Data Structures

#### Process
```
struct task_struct : Represent a process within a kernel.
struct fs_struct: Maintains association of process with struct FILE.
struct file_struct: Keeps info of open file.
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
struct vm_area_struct: Maintains individual summary of segments within a process. Eg code, data, 			  heap,mmap 
struct namespace: Data caching associated with inode.
struct page: Represent a portion of physical memory
```
#### Block Layer
```
struct request_queue: IO queue associated with device
struct request: Individual request within a request_queue
struct bio: Represents an unit of IO. A request_queue have many bio’s
struct bio_vec: Keeps 1) buffer info which is to be transferred, 2) start end of the buffer to be 				transferred. 3) struct page information 
struct gendisk: Represent a physical disk.
```

