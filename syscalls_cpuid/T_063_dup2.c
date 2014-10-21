#include "head.h"

void main()
{
	int fd;
	
	ASSERT((fd = open("testfile063", O_CREAT|O_RDWR)))	
	
	enter_syscall(63, 0)
	ASSERT(dup2(fd, 1))
	exit_syscall

	ASSERT(close(fd))
	ASSERT(unlink("testfile063"))
	return;
}
