#include "head.h"

#define socketcall(call, args) syscall(__NR_socketcall, call, args)

struct test_case_t {
	int call;
	unsigned long args[3];
	char *desc;
} TC[] = {
	{
		SYS_SOCKET, {
	PF_INET, SOCK_STREAM, 0}, "TCP stream"}, {
		SYS_SOCKET, {
	PF_UNIX, SOCK_DGRAM, 0}, "unix domain dgram"}, {
		SYS_SOCKET, {
	AF_INET, SOCK_RAW, 6}, "Raw socket"}, {
		SYS_SOCKET, {
	PF_INET, SOCK_DGRAM, 17}, "UDP dgram"}
};

void main()
{
	int i;
	
	for(i = 0;i < 4;i++) {
		enter_syscall(102, i)
		ASSERT(socketcall(TC[i].call, TC[i].args))
		exit_syscall
	}
}
