#include "head.h"

void main()
{
	int fd; 
	
	enter_syscall(60, 0)
	ASSERT(umask(022))	
	exit_syscall

	return;
}
