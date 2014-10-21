#include "head.h"

void main()
{
	ASSERT(creat("testfile010", 0444))

	enter_syscall(10,0)
	ASSERT(unlink("testfile010"))
	exit_syscall
	return;
}
