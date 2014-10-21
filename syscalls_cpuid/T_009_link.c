#include "head.h"

void main()
{
	ASSERT(creat("testfile009", 0444))
	
	enter_syscall(9,0)
	ASSERT(link("testfile009", "testfile009.s"))
	exit_syscall
	
	ASSERT(unlink("testfile009.s"))
	ASSERT(unlink("testfile009"))
	return;
}
