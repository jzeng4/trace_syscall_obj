#include "head.h"

void main()
{
	enter_syscall(11, 0)
	ASSERT(execve("./test", NULL, NULL))
	exit_syscall
	return;
}
