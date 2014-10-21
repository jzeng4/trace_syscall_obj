#include "head.h"

#ifndef TUNGETFEATURES
#define TUNGETFEATURES _IOR('T', 207, unsigned int)
#endif

void main()
{
	int netfd; 
	unsigned int features;

	ASSERT((netfd = open("/dev/net/tun", O_RDWR)))
	
	enter_syscall(54, 0)
	ASSERT(ioctl(netfd, TUNGETFEATURES, &features))
	exit_syscall
}
