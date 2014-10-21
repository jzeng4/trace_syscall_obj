#include "head.h"

void main()
{	
	enter_syscall(64, 0)
	ASSERT(getppid())
	exit_syscall

	return;
}
