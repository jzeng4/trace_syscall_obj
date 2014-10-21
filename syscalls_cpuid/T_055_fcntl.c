#include "head.h"

void main()
{
	int fd; 

	ASSERT((fd = open("testfile055", O_CREAT|O_RDWR)))	
	
	enter_syscall(55, 0)
	ASSERT(fcntl(fd, F_DUPFD, 1))
	exit_syscall
	
	ASSERT(unlink("testfile055"))
	return;
}
