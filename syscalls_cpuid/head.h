#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <unistd.h>
#include <utime.h>
#include <signal.h>
#include <pthread.h>
#include <ustat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/statfs.h>
#include <sys/resource.h> 
#include <sys/time.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <linux/net.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/mman.h>

#include "syscall.h"
#include "vmnotify.h"

#define ASSERT(fn)	\
	if(fn < 0) {	\
		assert(0);	\
	}

#define ASSERTNOTZ(fn) \
	if(fn == NULL) {	\
		assert(0);	\
	}

#define ASSERTGTZ(fn) \
	if(fn <= 0) {	\
		assert(0);	\
	}
