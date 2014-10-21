#include "head.h"

void main()
{
	struct utimbuf utim;
	ASSERT(creat("testfile030", 0444))
	
	enter_syscall(30, 0)
	ASSERT(utime("testfile030", &utim))
	exit_syscall
	
	ASSERT(unlink("testfile030"))
	return;
}
