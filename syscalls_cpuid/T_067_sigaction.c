#include "head.h"


void handler(int sig, siginfo_t * sip, void *ucp)
{
}

void main()
{
	struct sigaction sa;

	sa.sa_sigaction = handler;
	sa.sa_flags = SA_RESETHAND | SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, 0);

	enter_syscall(67, 0)
	ASSERT(sigaction(SIGUSR1, &sa, NULL))
	exit_syscall


	return;
}
