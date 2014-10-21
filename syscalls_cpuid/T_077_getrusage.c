#include "head.h"

void main()
{
	struct rusage usage;

	enter_syscall(77, 0)
	ASSERT(getrusage(RUSAGE_SELF, &usage))
	exit_syscall

	return;
}
