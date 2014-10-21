#include "head.h"

void main()
{
	ASSERT(creat("testfile033", 0444))
	
	enter_syscall(33,0)
	ASSERT(access("testfile033", F_OK))
	exit_syscall
	
	ASSERT(unlink("testfile033"))
	return;

}
