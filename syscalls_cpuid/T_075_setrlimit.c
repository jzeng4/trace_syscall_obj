#include "head.h"

void main()
{
	struct rlimit rl;
	
	ASSERT(getrlimit (RLIMIT_CPU, &rl))
	
	enter_syscall(75, 0)
 	ASSERT(setrlimit (RLIMIT_CPU, &rl))
	exit_syscall

	return;
}
