#include "head.h"

int main()
{
	int fd;

	ASSERT((fd = open("testfile094", O_CREAT|O_RDWR)))
	
	enter_syscall(94,0)
	ASSERT(fchmod(fd, 01777))
	exit_syscall
	
	ASSERT(unlink("testfile094"))

	return;
}
