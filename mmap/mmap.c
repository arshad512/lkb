#include <sys/types.h>
#include <sys/mman.h> /* mmap */
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Arshad Hussain <arshad.super@gmail.com
 */

#define DEVICE "/dev/mmapchar"

int main(void)
{
	int fd;
	char str[14] = "Fourteen";
	char *addr;

	/* This will call driver's open method */
	fd = open(DEVICE, O_RDWR);
	if (fd < 0) {
		/*
		 * Do check, backend mmap driver is loaded
		 */
		printf("open failed\n");
		printf("make sure insmod km.ko is done\n");
		return -1;
	}

	/* 
	 * Open success: A simple return 0 is sufficient to
	 * fake and make mmap() pass from user-space. But
	 * remember accessing will return a bus-error / core
	 * and fail.
	 *
	 * Proper vm_area_struct and allocation from
	 * remap_pfn_range() must be called from device driver
	 * to make the "Failing code" pass
	 *
	 */
	addr = (char*)mmap(NULL, 4096, PROT_READ|PROT_WRITE, 
		MAP_FILE|MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		printf("Failed to mmap\n");
		close(fd);
		return -1;
	}


	printf("ALL OK\n");

	/*
	 * Failing code: 2 lines.
	 * The below two line of code will only pass if the
	 * device driver implementation of mmap is done correctly.
	 * see code mmap/code.c for more.
	 */
	addr[0] = 'A';
	printf("Char = [%c]\n", addr[0]);

	/* This will call driver's release method */
	close(fd);
	return 0;
}

