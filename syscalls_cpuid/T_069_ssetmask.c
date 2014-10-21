#include "head.h"

void main()
{
	enter_syscall(69, 0)
	syscall(__NR_ssetmask, SIGALRM);
	exit_syscall

	return;
}
