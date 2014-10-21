#include "head.h"

void main()
{
	int fd; 
	
	enter_syscall(61, 0)
	ASSERT(chroot("/home"))
	exit_syscall

	return;
}
