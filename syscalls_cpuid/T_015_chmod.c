#include "head.h"

int main()
{
	ASSERT(creat("testfile015", 0444))
	
	enter_syscall(15,0)
	ASSERT(chmod("testfile015", 01777))
	exit_syscall
	
	ASSERT(unlink("testfile015"))

	return;
}
