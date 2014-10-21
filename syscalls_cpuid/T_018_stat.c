#include "head.h"

void main()
{
	struct stat buf;
	ASSERT(creat("testfile018", 0444))
	
	enter_syscall(18, 0)
	ASSERT(stat("testfile018", &buf))
	exit_syscall
	
	ASSERT(unlink("testfile018"))
	return;
}
