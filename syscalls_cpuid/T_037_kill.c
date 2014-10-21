#include "head.h"

void main()
{
	pid_t pid;
	int status;
	ASSERT((pid = fork()))
	
	if(pid == 0) {
		while(1);
	} else {
		enter_syscall(37, 0)
		ASSERT(kill(pid, SIGKILL))
		exit_syscall
		ASSERT(waitpid(pid, &status, 0))
	}
	return;
}
