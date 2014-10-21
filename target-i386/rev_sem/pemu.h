#ifndef PEMU_H
#define PEMU_H

#include <xed-interface.h>
//#include "PIN/pin.h"
//#include "qemu-pemu.h"
#include "qemu-common.h"
#include "cpu.h"
//#include "DISAS/disas.h"
//#include "PIN/pin_objs.h"
//#include "hashTable.h"


struct PEMU_EXEC_STATS;
struct PEMU_INST;
#if 0
struct PEMU_HOOK_FUNCS;
extern struct PEMU_EXEC_STATS pemu_exec_stats;
extern struct PEMU_HOOK_FUNCS pemu_hook_funcs;
extern struct PEMU_BBL pemu_bbl;
#endif

extern struct PEMU_EXEC_STATS pemu_exec_stats;
extern struct PEMU_INST pemu_inst;

struct PEMU_EXEC_STATS {
	char PEMU_binary_name[100];
	char PEMU_plugin_name[100];
	uint32_t PEMU_start;
	uint32_t PEMU_already_flush;
	uint32_t PEMU_cr3;
	
	target_ulong PEMU_task_addr;
	target_ulong PEMU_exec_pc;
	target_ulong PEMU_dis_pc;
	uint32_t PEMU_pid;	
	target_ulong PEMU_main_start;
	target_ulong PEMU_img_start;
	target_ulong PEMU_img_end;
	target_ulong PEMU_libc_start;
	target_ulong PEMU_libc_end;
	target_ulong PEMU_iret_target_pc;

	int PEMU_int_level;
	uint32_t PEMU_kern_esp;
	int PEMU_start_trace_syscall;
	int PEMU_syscall_num;
	uint32_t PEMU_in_schedule;
};

struct PEMU_INST {
	target_ulong PEMU_inst_pc;
	xed_state_t PEMU_dstate;
	xed_decoded_inst_t PEMU_xedd_g;
	char PEMU_inst_buf[15];
	char PEMU_inst_str[128];
};

#if 0
struct PEMU_BBL {
	target_ulong PEMU_bbl_pc;
	//xed_decoded_inst_t PEMU_xedd_g;
	BBL bbl;
};

struct PEMU_HOOK_FUNCS {
	//TODO
    INS_INSTRUMENT_CALLBACK inst_hook;
	BBL_INSTRUMENT_CALLBACK bbl_hook;
	SYSCALL_ENTRY_CALLBACK enter_syscall_hook;
	SYSCALL_EXIT_CALLBACK exit_syscall_hook;
};
#endif


int PEMU_init(void*);
int PEMU_exit(void);
#endif
