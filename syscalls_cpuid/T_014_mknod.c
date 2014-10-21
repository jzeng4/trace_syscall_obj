#include "head.h"


#define master 0x00010000
void main()
{
	char fn[]="char.spec";

	enter_syscall(14,0)
  	ASSERT(mknod(fn, S_IFCHR|S_IRUSR|S_IWUSR, master|0x0001))
	exit_syscall
	
	ASSERT(unlink(fn))
	return;
}
