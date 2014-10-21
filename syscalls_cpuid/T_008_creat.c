#include "head.h"

void main()
{
	enter_syscall(8,0)
	ASSERT(creat("testfile008", 0444))
	exit_syscall

	ASSERT(unlink("testfile008"))
	return;

}
