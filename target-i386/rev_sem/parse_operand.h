#ifndef PARSE_OPRAND_H
#define PARSE_OPRAND_H

#include "pemu.h"

extern uint32_t s_base, s_index, s_scale, s_dis;
extern xed_reg_enum_t s_base_regid, s_index_regid;



int operand_is_reg(const xed_operand_enum_t op_name, xed_reg_enum_t * reg_id);
int operand_is_relbr(const xed_operand_enum_t op_name, uint32_t * branch);
int operand_is_mem(const xed_operand_enum_t op_name, uint32_t* mem_addr, 
		   int operand_i);
int operand_is_imm(const xed_operand_enum_t op_name, uint32_t *value);
int get_mem_operand_size(const xed_operand_enum_t op_name, uint32_t operand_i);

uint32_t get_callDest(const xed_decoded_inst_t* xedd, uint32_t cur_pc);

#endif
