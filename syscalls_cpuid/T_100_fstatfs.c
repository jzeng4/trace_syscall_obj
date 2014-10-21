#include "head.h"

void main()
{
	struct statfs buf;
	int fd;

	ASSERT((fd = open("testfile100", O_CREAT|O_RDWR)))

	enter_syscall(100, 0)
	ASSERT(fstatfs(fd, &buf))
	exit_syscall
	
	ASSERT(unlink("testfile100"))
	return;
}
