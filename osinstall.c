/*
 * 	filename: osinstall.c
 *	author:	  bodhix
 *	date:	  2017-05-04
 *	desc:	  check whether a disk has installed an os on it.
 *
 *			  When os is installed, the first 512 bytes would not be 0
 *			  since main boot record(MBR) is written.
 *			  Additionally, content of the last 2 bytes of the bootsector is 0xaa55
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define linfo(...) do {\
	printf("[INFO] "); \
	printf(__VA_ARGS__); \
	printf("\n"); \
} while(0)

#define lerror(...) do {\
	printf("[ERROR] "); \
	printf(__VA_ARGS__); \
	printf("\n"); \
} while(0)

void usage()
{
	linfo("usage: osinstall disk");
}	

#define OFFSET 		510
#define CHECK_LEN	2                  

// check whether the sector is finished with 0xaa55
int check_boot_sector(const char* dev)
{
	assert(dev);
	char buf[CHECK_LEN] = {0};
	unsigned short valid_end = 0xaa55;
	unsigned short got_end = 0;

	int fd = open(dev, O_RDONLY);
	if (fd < 0)
	{
		lerror("open %s failed: %s", dev, strerror(errno));
		return -1;
	}

	off_t offset = lseek(fd, OFFSET, SEEK_SET);
	if (offset == -1)
	{
		lerror("lseek %s failed: %s", dev, strerror(errno));
		return -1;
	}

	ssize_t res = read(fd, buf, CHECK_LEN);
	if (res < 0)
	{
		lerror("read %s failed: %s", dev, strerror(errno));
		return -1;
	}

	close(fd);

	// begin to check
	memcpy(&got_end, buf, 2);
	if (got_end == valid_end)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void check_has_install_os(const char* dev)
{
	assert(dev);
	int res = check_boot_sector(dev);
	if (res == 1)
	{
		linfo("disk [%s] has os installed on it", dev);
	}
	else if (res == 0)
	{
		linfo("disk [%s] has not os installed on it", dev);
	}
	else
	{
		lerror("check disk [%s] failed", dev);
	}
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		usage();
		return -1;
	}

	char *dev = (char*)argv[1];
	check_has_install_os(dev);
}