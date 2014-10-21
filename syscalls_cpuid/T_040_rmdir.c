#include "head.h"

void main()
{
	char path[100];
	char wpath[100];

	ASSERTNOTZ(getcwd(path, 100))
	sprintf(wpath, "%s/testfolder040", path);
	ASSERT(mkdir(wpath, 0777))
	
	enter_syscall(40,0)
	ASSERT(rmdir(wpath))
	exit_syscall
	
	return;
}
