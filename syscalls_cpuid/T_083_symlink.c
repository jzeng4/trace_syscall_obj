#include "head.h"

void main()
{
	ASSERT(creat("testfile083", 0444))
	
	enter_syscall(83,0)
	ASSERT(symlink("testfile083", "testfile083.new"))
	exit_syscall

	ASSERT(unlink("testfile083.new"))
	ASSERT(unlink("testfile083"))
	return;

}
