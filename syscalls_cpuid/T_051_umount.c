#include "head.h"

void main()
{
   const char* src  = "none";
   const char* trgt = "/var/tmp";
   const char* type = "tmpfs";
   const unsigned long mntflags = 0;
   const char* opts = "mode=0700,uid=65534";   /* 65534 is the uid of nobody */

   ASSERT(mount(src, trgt, type, mntflags, opts))

   enter_syscall(51, 0)
   ASSERT(umount(trgt))
   exit_syscall
   
   return;
}
