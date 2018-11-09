#include <sys/types.h>
#include <sys/mman.h> /* mmap */
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

/*
 * Arshad Hussain <arshad.super@gmail.com
 */

#define DEVICE "/dev/mmapchar"
#define QUERY_GET_VARIABLES _IOR('q', 1, int *)
#define QUERY_CLR_VARIABLES _IO('q', 2)
#define QUERY_SET_VARIABLES _IOW('q', 3, int *)

int main(void)
{
	int fd;
	int q = 0;

	/* This will call driver's open method */
	fd = open(DEVICE, O_RDWR);
	if (fd < 0) {
		/*
		 * Do check, backend mmap driver is loaded
		 */
		printf("open failed\n");
		printf("make sure insmod code.ko is done\n");
		return -1;
	}

	/*
	 * Read
	 */
#if 0
	if (ioctl(fd, QUERY_GET_VARIABLES, &q) == -1)
	{
		perror("query_apps ioctl get");
	}
	printf("five ... done...%d\n",q);
	
	if (ioctl(fd, QUERY_CLR_VARIABLES, &q) == -1)
	{
		perror("query_apps ioctl get");
	}
	printf("five ... done...CLR\n");
#endif

	/*
	 * Write
	 */
	q = 140;
	
	if (ioctl(fd, QUERY_SET_VARIABLES, &q) == -1)
	{
		perror("error ioctl get");
	}
	printf("five ... done...SET\n");

	/* This will call driver's release method */
	close(fd);
	return 0;
}

