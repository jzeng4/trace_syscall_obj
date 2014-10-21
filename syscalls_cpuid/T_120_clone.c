#include "head.h"

void main()
{
	enter_syscall(120, 0)
	ASSERT(fork())
	exit_syscall
}
