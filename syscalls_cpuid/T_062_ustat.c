#include "head.h"

void main()
{
	struct ustat ubuf;
	struct stat buf;

	ASSERT(stat("/", &buf))
	
	enter_syscall(62, 0)
	ASSERT(ustat(buf.st_dev, &ubuf))	
	exit_syscall

	return;
}
