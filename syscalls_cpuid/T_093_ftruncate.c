#include "head.h"

void main()
{
	int fd;
	char buf[1024];
	
	ASSERT((fd = open("testfile093", O_CREAT|O_RDWR)))
	ASSERTGTZ(write(fd, buf, 1024))
	
	enter_syscall(93, 0)
	ASSERT(ftruncate(fd, 512))
	exit_syscall
	
	ASSERT(close(fd))
	ASSERT(unlink("testfile093"))
	return;
}
