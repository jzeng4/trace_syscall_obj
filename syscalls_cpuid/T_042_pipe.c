#include "head.h"

void main()
{
	int fd[2];
	
	ASSERT((fd[0] = open("testfile42", O_CREAT|O_RDWR)))
	fd[1] = 0;
	
	enter_syscall(42, 0)
	ASSERT(pipe(fd))
	exit_syscall

	ASSERT(close(fd[0]))
	ASSERT(unlink("testfile42"))
	return;
}
