#include "head.h"

void main()
{
	pid_t pid;
	
	ASSERT((pid = getpid()))

	enter_syscall(96, 0)
	ASSERT(getpriority(PRIO_PROCESS, pid))	
	exit_syscall

	return;
}
