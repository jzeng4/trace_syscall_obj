#include "head.h"

void main()
{
	pid_t pid;
	int status;
	ASSERT((pid = fork()))
	
	if(pid == 0) {
		while(1);
	} else {
		ASSERT(kill(pid, SIGKILL))
		enter_syscall(7, 0)
		ASSERT(waitpid(pid, &status, 0))
		exit_syscall
	}
	return;
}
