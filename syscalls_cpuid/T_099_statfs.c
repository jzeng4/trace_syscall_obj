#include "head.h"

void main()
{
	struct statfs buf;
	int fd;

	ASSERT(creat("testfile099", 0444))

	enter_syscall(99,0)
	ASSERT(statfs("testfile099", &buf))
	exit_syscall
	
	ASSERT(unlink("testfile099"))
	return;
}
