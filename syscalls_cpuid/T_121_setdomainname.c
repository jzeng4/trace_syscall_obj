#include "head.h"

void main()
{
	char *test_domain_name = "test_dom";
	char old_domain_name[1024];

	ASSERT(getdomainname(old_domain_name, sizeof(old_domain_name)))
	
	enter_syscall(121, 0)
	ASSERT(setdomainname(test_domain_name, sizeof(test_domain_name)))
	exit_syscall

	ASSERT(setdomainname(old_domain_name, strlen(old_domain_name)))
}
