#include "head.h"

void main()
{
	enter_syscall(199,0)
	ASSERT(getuid())
	exit_syscall
	return;
}
