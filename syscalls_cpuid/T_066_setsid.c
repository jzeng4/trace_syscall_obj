#include "head.h"

void main()
{
	int fd; 
	
	enter_syscall(66, 0)
	ASSERT(setsid())	
	exit_syscall

	return;
}
