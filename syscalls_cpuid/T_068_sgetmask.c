#include "head.h"


void main()
{
	enter_syscall(68, 0)
	syscall(__NR_sgetmask);
	exit_syscall


	return;
}
