#include "head.h"

void main()
{
	enter_syscall(42, 0)
	ASSERT(brk((void*)0x100))
	exit_syscall
	return;
}
