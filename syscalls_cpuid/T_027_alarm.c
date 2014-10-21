#include "head.h"

void main()
{
   enter_syscall(27, 0)
   ASSERT(alarm(1))
   exit_syscall

   return;
}
