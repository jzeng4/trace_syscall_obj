#include "head.h"

void main()
{
	int fd; 
	ASSERT((fd = open("testfile019",O_CREAT|O_RDWR)))
	
	enter_syscall(19,0)
	ASSERT(lseek(fd, 2, SEEK_SET))
	exit_syscall

	ASSERT(close(fd))
	ASSERT(unlink("testfile019"))
	return;
}
