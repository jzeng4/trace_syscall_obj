#include "head.h"

void main()
{
	int fd; 
	
	ASSERT((fd = open("testfile006",O_CREAT|O_RDWR)))	

	enter_syscall(6,0)
	ASSERT(close(fd))
	exit_syscall

	ASSERT(unlink("testfile006"))
	return;
}
