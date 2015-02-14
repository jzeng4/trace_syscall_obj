#include "pemu.h"
//#include "PIN/pin_qemu.h"
//#include "pemu_helper.h"
#include <stdio.h>
//#include "DISAS/disas.h"
//#include "Disas/hashTable.h"

struct PEMU_EXEC_STATS pemu_exec_stats;
struct PEMU_INST pemu_inst;

#if 0
struct PEMU_HOOK_FUNCS pemu_hook_funcs;
struct PEMU_BBL pemu_bbl;
#endif

void init_inst(void)
{
	 xed_tables_init();
	 xed_state_zero(&pemu_inst.PEMU_dstate);

  	 xed_state_init(&pemu_inst.PEMU_dstate,
			 XED_MACHINE_MODE_LEGACY_32,
			 XED_ADDRESS_WIDTH_32b, XED_ADDRESS_WIDTH_32b);

}

#if 0
void init_bbl(void)
{
	memset(&pemu_bbl, 0, sizeof(struct PEMU_BBL));
	pemu_bbl.bbl = (BBL) malloc(sizeof(struct Bbl));
}

inline void inst_update(char *buf)
{
	xed_decoded_inst_zero_set_mode(&pemu_inst.PEMU_xedd_g, &pemu_inst.PEMU_dstate);
	xed_error_enum_t xed_error = xed_decode(&pemu_inst.PEMU_xedd_g,
			XED_STATIC_CAST(const xed_uint8_t *,  buf), 15);
	if (xed_error == XED_ERROR_NONE) 
	{
	   xed_decoded_inst_dump_intel_format(&pemu_inst.PEMU_xedd_g, pemu_inst.PEMU_inst_str, 
			   sizeof(pemu_inst.PEMU_inst_str), 0);	
	   //pemu_xed.PEMU_g_xi = xed_decoded_inst_inst(&pemu_xed.PEMU_xedd_g);
   	   //pemu_xed.PEMU_op = xed_inst_operand(pemu_xed.PEMU_g_xi, 0);
   	   //pemu_xed.PEMU_op_name = xed_operand_name(pemu_xed.PEMU_op);
	   //pemu_xed.PEMU_opcode = xed_decoded_inst_get_iclass(&pemu_xed.PEMU_xedd_g);
	}
}
#endif

int PEMU_init(void *env)
{
	//pemu_exec_stats.PEMU_hook_sys_call = -1;
	//init_pin_regmapping(env);
	//taintInit();
	init_inst();
	//load_syscalls();
	//load_modules();
	open_database();
	load_function_interface();
	taintInit2();
	//init_bbl();
	//init_hashTable();
	//init_disas();
	//setup_inst_hook();
	//setup_inst_hook2();
}

#if 0
int PEMU_exit(void)
{
	pemu_exec_stats.PEMU_start = 0;
	return 1;
}
#endif
