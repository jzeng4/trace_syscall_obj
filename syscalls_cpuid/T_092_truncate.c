#include "head.h"

void main()
{
	int fd;
	char buf[1024];
	
	ASSERT((fd = open("testfile092", O_CREAT|O_RDWR)))
	ASSERTGTZ(write(fd, buf, 1024))
	ASSERT(close(fd))
	ASSERT(chmod("testfile092", 01777))

	enter_syscall(92, 0)
	ASSERT(truncate("./testfile092", 512))
	exit_syscall
	
	ASSERT(unlink("testfile092"))
	return;
}
