/* ©2011 Massachusetts Institute of Technology */

//#if 0
#define vm_mark_buf_in(x,y) \
    { \
    void *b = x; \
    int n = y; \
    asm __volatile__ ("push %%eax\n\t" \
         "push %%ebx\n\t" \
         "push %%ecx\n\t" \
         "push %%edx\n\t" \
         "mov $0, %%eax\n\t" \
         "mov $0xdeadbeef, %%ebx\n\t" \
         "mov %0, %%ecx\n\t" \
         "mov %1, %%edx\n\t" \
         "cpuid\n\t" \
         "pop %%edx\n\t" \
         "pop %%ecx\n\t" \
         "pop %%ebx\n\t" \
         "pop %%eax\n\t" \
         : \
         : "r" (b), \
           "r" (n) \
         : "%eax", "%ebx", "%ecx", "%edx" \
    ); \
    }

#define vm_mark_buf_out(x,y) \
    { \
    void *b = x; \
    int n = y; \
    asm __volatile__ ("push %%eax\n\t" \
         "push %%ebx\n\t" \
         "push %%ecx\n\t" \
         "push %%edx\n\t" \
         "mov $0, %%eax\n\t" \
         "mov $0xdeadf00d, %%ebx\n\t" \
         "mov %0, %%ecx\n\t" \
         "mov %1, %%edx\n\t" \
         "cpuid\n\t" \
         "pop %%edx\n\t" \
         "pop %%ecx\n\t" \
         "pop %%ebx\n\t" \
         "pop %%eax\n\t" \
         : \
         : "r" (b), \
           "r" (n) \
         : "%eax", "%ebx", "%ecx", "%edx" \
    ); \
    }
//#endif

//#define vm_mark_buf_in(x,y)
//#define vm_mark_buf_out(x,y)

#define enter_syscall(x,y)	\
	unsigned int args;	\
	args = (y << 16) | x;	\
	vm_mark_buf_in(0x12345678,args)

#define exit_syscall	\
	vm_mark_buf_out(0x12345678, 0)
	
