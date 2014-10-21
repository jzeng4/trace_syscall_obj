#include "head.h"

void main()
{
	char buffer[1024];

	ASSERT(creat("testfile085", 0444))
	ASSERT(symlink("testfile085", "testfile085.new"))
	
	enter_syscall(85,0)
	ASSERT(readlink("testfile085.new", buffer, sizeof(buffer)))
	exit_syscall

	ASSERT(unlink("testfile085.new"))
	ASSERT(unlink("testfile085"))
	return;

}
