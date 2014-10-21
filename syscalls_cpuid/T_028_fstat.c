#include "head.h"

void main()
{
	struct stat buf;
	int fd;
	
	ASSERT((fd = open("testfile028", O_CREAT|O_RDWR)))
	
	enter_syscall(28, 0)
	ASSERT(fstat(fd, &buf))
	exit_syscall
	
	ASSERT(close(fd))
	ASSERT(unlink("testfile028"))
	return;
}
