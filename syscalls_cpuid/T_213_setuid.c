#include "head.h"

void main()
{
	enter_syscall(213,0)
	ASSERT(setuid(1000))
	exit_syscall
	return;
}
