#include "head.h"

void main()
{
	char path[100];
	char wpath[100];

	ASSERTNOTZ(getcwd(path, 100))
	sprintf(wpath, "%s/testfolder012", path);
	ASSERT(mkdir(wpath, 0777))
	
	enter_syscall(12,0)
	ASSERT(chdir(wpath))
	exit_syscall
	
	ASSERT(chdir(path))
	ASSERT(rmdir(wpath))

	return;
}
