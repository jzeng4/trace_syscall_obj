#include "head.h"

void main()
{
	int fd;
	char *str;
	
	ASSERT((fd = open("testfile192", O_CREAT|O_RDWR)))

	enter_syscall(192, 0)
	ASSERT((str = mmap(0, 1024*1024, PROT_READ, MAP_PRIVATE, (int)fd, 0)))
	exit_syscall
	
	ASSERT(munmap(str,sizeof(str)))

	ASSERT(close(fd))
	ASSERT(unlink("testfile192"))
	return;
}

