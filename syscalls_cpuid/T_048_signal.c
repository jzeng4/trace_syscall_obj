#include "head.h"

pid_t pid;

void handler()
{
	ASSERT(kill(pid, SIGKILL))
}

void main()
{
	int status;
	ASSERT((pid = fork()))
	
	if(pid == 0) {
		while(1);
	} else {
		enter_syscall(48, 0)
		ASSERT(signal(SIGALRM, handler))
		exit_syscall
	}
	return;
}
