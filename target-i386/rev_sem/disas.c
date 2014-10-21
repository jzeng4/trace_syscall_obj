#include "disas.h"
//#include "../hashTable.h"


xed_error_enum_t disas_one_inst_ex(target_ulong pc, struct PEMU_INST *inst)
{
	PEMU_read_mem(pc, 15, inst->PEMU_inst_buf);
	xed_decoded_inst_zero_set_mode(&inst->PEMU_xedd_g, &inst->PEMU_dstate);
	xed_error_enum_t xed_error = xed_decode(&inst->PEMU_xedd_g,
			XED_STATIC_CAST(const xed_uint8_t *, inst->PEMU_inst_buf), 15);
	xed_decoded_inst_dump_att_format(&inst->PEMU_xedd_g, inst->PEMU_inst_str, sizeof(inst->PEMU_inst_str), 0);
	return xed_error;	
}

xed_error_enum_t disas_callnear_ex(target_ulong pc, struct PEMU_INST *inst)
{
	PEMU_read_mem(pc, 15, inst->PEMU_inst_buf);
	if(pemu_inst.PEMU_inst_buf[0] != (char)0xe8 
			&& pemu_inst.PEMU_inst_buf[0] != (char)0xff) {
		return XED_ERROR_LAST;
	}

	xed_decoded_inst_zero_set_mode(&inst->PEMU_xedd_g, &inst->PEMU_dstate);
	xed_error_enum_t xed_error = xed_decode(&inst->PEMU_xedd_g,
			XED_STATIC_CAST(const xed_uint8_t *, inst->PEMU_inst_buf), 15);
	//xed_decoded_inst_dump_att_format(&inst->PEMU_xedd_g, inst->PEMU_inst_str, sizeof(inst->PEMU_inst_str), 0);
	return xed_error;	
}

int disas_is_jmpE9_call(target_ulong pc)
{
	char buf[15];
	PEMU_read_mem(pc, 15, buf);
	if(buf[0] == (char)0xe9 || buf[0] == (char)0xe8 || buf[0] == (char)0xff) {
		return 1;
	} else {
		return 0;
	}

}

int disas_is_call(target_ulong pc)
{
	char buf[15];
	PEMU_read_mem(pc, 15, buf);
	if(buf[0] == (char)0xe8 || buf[0] == (char)0xff) {
		return 1;
	} else {
		return 0;
	}

}
