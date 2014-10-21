#include "head.h"

void main()
{
	int fd; 
	
	enter_syscall(5, 0)
	ASSERT((fd = open("testfile005", O_CREAT|O_RDWR)))	
	exit_syscall

	ASSERT(close(fd))
	ASSERT(unlink("testfile005"))
	return;
}
