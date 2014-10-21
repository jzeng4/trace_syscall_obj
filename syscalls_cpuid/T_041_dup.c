#include "head.h"

void main()
{
	int fd; 
	
	ASSERT((fd = open("testfile041", O_CREAT|O_RDWR)))	
	
	enter_syscall(41, 0)
	ASSERT(dup(fd))
	exit_syscall

	ASSERT(close(fd))
	ASSERT(unlink("testfile041"))
	return;
}
