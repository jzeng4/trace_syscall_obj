#include "head.h"

void main()
{
	struct stat buf;
	ASSERT(creat("testfile084", 0444))
	
	enter_syscall(18, 0)
	ASSERT(lstat("testfile084", &buf))
	exit_syscall
	
	ASSERT(unlink("testfile084"))
	return;
}
