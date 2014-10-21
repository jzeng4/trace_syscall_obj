#include "head.h"

void main()
{
	struct rlimit rl;
	
	enter_syscall(191, 0)
	ASSERT(getrlimit(RLIMIT_CPU, &rl))
	exit_syscall
 	
	ASSERT(setrlimit(RLIMIT_CPU, &rl))

	return;
}
