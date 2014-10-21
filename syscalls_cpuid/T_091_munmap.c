#include "head.h"

void main()
{
	int fd;
	char *str;
	
	ASSERT((fd = open("testfile091", O_CREAT|O_RDWR)))

	ASSERT((str = mmap(0, 1024*1024, PROT_READ, MAP_PRIVATE, (int)fd, 0)))
	
	enter_syscall(91, 0)
	ASSERT(munmap(str,sizeof(str)))
	exit_syscall

	ASSERT(close(fd))
	ASSERT(unlink("testfile091"))
	return;
}

