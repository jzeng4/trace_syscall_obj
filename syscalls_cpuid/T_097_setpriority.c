#include "head.h"

void main()
{
	pid_t pid;
	
	ASSERT((pid = getpid()))
	ASSERT(nice(7))

	enter_syscall(97, 0)
	ASSERT(setpriority(PRIO_PROCESS, pid, 8))
	exit_syscall

	return;
}
