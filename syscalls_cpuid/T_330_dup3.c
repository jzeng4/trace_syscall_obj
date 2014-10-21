#include "head.h"

void main()
{
	int fd, fd1;
	
	ASSERT((fd = open("testfile330", O_CREAT|O_RDWR)))	
	
	enter_syscall(330, 0)
	ASSERT(dup3(fd, 1, 0))
	exit_syscall

	ASSERT(close(fd))
	ASSERT(unlink("testfile330"))
	return;
}
