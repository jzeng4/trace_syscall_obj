#include <xed-interface.h>
#include "cpu.h"
#include "hook_inst.h"
//#include "ds_code/call_stack.h"
#include "parse_operand.h"
#include "linux.h"
#include "heap_shadow.h"
//#include "ds_code/call_stack.h"
#include "condition.h"
#include "config_pemu.h"

#define DEBUG_TAINT
#define ENABLE_TAINT

#include "taint.h"

static uint32_t g_pc;
static char g_inst_str[500];
static char g_inst_buffer[15];
static char g_inst_name[1024];
static xed_iclass_enum_t g_opcode;


static void ds_code_handle_mem_access1(INS ins)
{
	uint32_t addr;
	NodeType *p;
	const xed_operand_t *op_1 = xed_inst_operand(ins, 1);
	xed_operand_enum_t op_name_1 = xed_operand_name(op_1);

//	if(is_special_inst(&pemu_inst.PEMU_xedd_g)) {
//		return;
//	}
	if(operand_is_mem(op_name_1, &addr, 1)) {
		int size = xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 1);
		if(p = ds_code_rbtFind2(addr)) {
			//printf("1: pc:%x str:%s base:%x addr:%x p_key:%x p_size:%x new_size:%x mem_size:%x\n", g_pc, g_inst_str, s_base, addr, p->key, p->size, addr-p->key, size);
			p->size = addr-p->key + size + 1;
			update_traced_size(p->type, p->size);
		}
	}
}

static void ds_code_handle_mem_access0(INS ins)
{
	uint32_t addr;
	NodeType *p;
	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	if(operand_is_mem(op_name_0, &addr, 0)) {
		int size = xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 0);
		if(p = ds_code_rbtFind2(addr)) {
			//printf("0: pc:%x str:%s base:%x addr:%x p_key:%x p_size:%x new_size:%x mem_size:%x\n", g_pc, g_inst_str, s_base, addr, p->key, p->size, addr-p->key, size);
			p->size = addr-p->key + size + 1;
			update_traced_size(p->type, p->size);
		}
	}
}
//#endif

void infer_size(uint32_t pc)
{
	PEMU_read_mem(pc, 15, pemu_inst.PEMU_inst_buf);
	xed_decoded_inst_zero_set_mode(&pemu_inst.PEMU_xedd_g, &pemu_inst.PEMU_dstate);
	xed_error_enum_t xed_error = xed_decode(&pemu_inst.PEMU_xedd_g,
			XED_STATIC_CAST(const xed_uint8_t *, pemu_inst.PEMU_inst_buf), 15);

	xed_iclass_enum_t opcode = g_opcode = xed_decoded_inst_get_iclass(&pemu_inst.PEMU_xedd_g);
	xed_decoded_inst_dump_att_format(&pemu_inst.PEMU_xedd_g, g_inst_str, sizeof(g_inst_str), 0);
	g_pc = pc;
	INS ins = xed_decoded_inst_inst(&pemu_inst.PEMU_xedd_g);
	ds_code_handle_mem_access0(ins);
	ds_code_handle_mem_access1(ins);
}
