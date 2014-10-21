#include "head.h"

void main()
{
	char path[100];
	char wpath[100];

	ASSERTNOTZ(getcwd(path, 100))
	sprintf(wpath, "%s/testfolder039", path);

	enter_syscall(39,0)
	ASSERT(mkdir(wpath, 0777))
	exit_syscall

	ASSERT(rmdir(wpath))
	return;	
}
