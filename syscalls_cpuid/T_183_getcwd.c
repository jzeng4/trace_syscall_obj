#include "head.h"

void main()
{
	char path[100];

	enter_syscall(183,0)
	ASSERTNOTZ(getcwd(path, 100))
	exit_syscall
}

