#include "head.h"


void main()
{
	enter_syscall(20,0)
	ASSERT(getpid())
	exit_syscall
	return;
}
