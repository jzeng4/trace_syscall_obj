#include "head.h"

void main() {
	void * mydata;
	char * hellostr = "Hello This Is Read Only";
	
	int pagesize = sysconf(_SC_PAGE_SIZE);

	ASSERTGTZ((mydata = memalign(pagesize, pagesize)))
	strncpy(mydata, hellostr, strlen(hellostr) + 1);

	enter_syscall(125, 0)
	ASSERT(mprotect(mydata, pagesize, PROT_READ))
	exit_syscall

	return;
}

