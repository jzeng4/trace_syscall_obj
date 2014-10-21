#include "head.h"


void main()
{
	struct utimbuf utim;
	ASSERT(creat("testfile038", 0444))
	
	enter_syscall(38, 0)
	ASSERT(rename("testfile038", "new"))
	exit_syscall
	
	ASSERT(unlink("new"))
	return;
}
