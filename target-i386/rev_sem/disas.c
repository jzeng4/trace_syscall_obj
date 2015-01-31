#include "disas.h"
#include "heap_shadow.h"
//#include "../hashTable.h"


int operand_is_mem4(const xed_operand_enum_t op_name, uint32_t * mem_addr,
		    int operand_i)
{

	xed_reg_enum_t basereg;
	//mem_taint = 0;

	switch (op_name) {
		/* Memory */
	case XED_OPERAND_AGEN:
	case XED_OPERAND_MEM0:
	case XED_OPERAND_MEM1:{
			unsigned long base = 0;
			unsigned long index = 0;
			unsigned long scale = 1;
			unsigned long segbase = 0;
			unsigned short segsel = 0;
			unsigned long displacement = 0;

			/* Set memory index */
			int mem_idx = 0;
			if (op_name == XED_OPERAND_MEM1)
				mem_idx = 1;

			/* Initialization */
			base = 0;
			index = 0;
			scale = 0;
			segbase = 0;
			segsel = 0;
			displacement = 0;

			segbase = 0;
			// Get Base register
			xed_reg_enum_t base_regid =
			    xed_decoded_inst_get_base_reg(&pemu_inst.PEMU_xedd_g, mem_idx);

			if (base_regid != XED_REG_INVALID) {
				base = PEMU_get_reg(base_regid);
			}
			// Get Index register and Scale
			xed_reg_enum_t index_regid =
			    xed_decoded_inst_get_index_reg(&pemu_inst.PEMU_xedd_g, mem_idx);
			if (mem_idx == 0 && index_regid != XED_REG_INVALID) {
				index = PEMU_get_reg(index_regid);

				if (xed_decoded_inst_get_scale
				    (&pemu_inst.PEMU_xedd_g, operand_i) != 0) {
					scale = (unsigned long)
					    xed_decoded_inst_get_scale(&pemu_inst.PEMU_xedd_g,
								       mem_idx);
				}
			}
			//Get displacement
			displacement = (unsigned long)
			    xed_decoded_inst_get_memory_displacement(&pemu_inst.PEMU_xedd_g,
								     mem_idx);
			*mem_addr =
			    segbase + base + index * scale + displacement;
			return 1;
		}

	default:
		return 0;
	}

}


static int operand_is_reg(const xed_operand_enum_t op_name, xed_reg_enum_t * reg_id) 
{
	switch (op_name) {
		/* Register */
	case XED_OPERAND_REG0:
	case XED_OPERAND_REG1:
	case XED_OPERAND_REG2:
	case XED_OPERAND_REG3:
	case XED_OPERAND_REG4:
	case XED_OPERAND_REG5:
	case XED_OPERAND_REG6:
	case XED_OPERAND_REG7:
	case XED_OPERAND_REG8:
	case XED_OPERAND_REG9:
	case XED_OPERAND_REG10:
	case XED_OPERAND_REG11:
	case XED_OPERAND_REG12:
	case XED_OPERAND_REG13:
	case XED_OPERAND_REG14:
	case XED_OPERAND_REG15:{
			*reg_id = xed_decoded_inst_get_reg(&pemu_inst.PEMU_xedd_g, op_name);
			return 1;
		} default:return 0;
	}
}
static int operand_is_mem(const xed_operand_enum_t op_name, uint32_t* mem_addr, 
		   int operand_i)
{

//	xed_reg_enum_t basereg;	

	switch (op_name) {
		/* Memory */
	case XED_OPERAND_AGEN:
	case XED_OPERAND_MEM0:
	case XED_OPERAND_MEM1:{
			unsigned long base = 0;
			unsigned long index = 0;
			unsigned long scale = 1;
			unsigned long segbase = 0;
			unsigned short segsel = 0;
			unsigned long displacement = 0;
//			size_t remaining = 0;

			/* Set memory index */
			int mem_idx = 0;
			if (op_name == XED_OPERAND_MEM1)
				mem_idx = 1;

//			unsigned int memlen =
//			    xed_decoded_inst_operand_length(&PEMU_xedd_g, operand_i);

			/* Initialization */
			base = 0;
			index = 0;
			scale = 0;
			segbase = 0;
			segsel = 0;
			displacement = 0;

			xed_reg_enum_t seg_regid =
			    xed_decoded_inst_get_seg_reg(&pemu_inst.PEMU_xedd_g, mem_idx);

			if (seg_regid != XED_REG_INVALID)
				segbase =PEMU_get_seg(seg_regid);

			// Get Base register
			xed_reg_enum_t base_regid =
			    xed_decoded_inst_get_base_reg(&pemu_inst.PEMU_xedd_g, mem_idx);
			
			if (base_regid != XED_REG_INVALID) {
				base = PEMU_get_reg(base_regid);
			}
			// Get Index register and Scale
			xed_reg_enum_t index_regid =
			    xed_decoded_inst_get_index_reg(&pemu_inst.PEMU_xedd_g, mem_idx);
			if (mem_idx == 0 && index_regid != XED_REG_INVALID) {
				index = PEMU_get_reg(index_regid);

				if (xed_decoded_inst_get_scale
				    (&pemu_inst.PEMU_xedd_g, operand_i) != 0) {
					scale =
					    (unsigned long)
					    xed_decoded_inst_get_scale(&pemu_inst.PEMU_xedd_g,
								       mem_idx);
				}
			}
			displacement =
			    (unsigned long)
			    xed_decoded_inst_get_memory_displacement(&pemu_inst.PEMU_xedd_g,
								     mem_idx);
			*mem_addr =
			    segbase + base + index * scale + displacement;

			return 1;
		}

	default:
		return 0;
	}

}



static target_ulong Instrument_CALL_NEAR(target_ulong pc)
{
	uint32_t mem_addr;
	xed_reg_enum_t  reg_id;
	target_ulong target;

	PEMU_read_mem(pc, 15, pemu_inst.PEMU_inst_buf);
	xed_decoded_inst_zero_set_mode(&pemu_inst.PEMU_xedd_g, &pemu_inst.PEMU_dstate);
	xed_error_enum_t xed_error = xed_decode(&pemu_inst.PEMU_xedd_g,
			XED_STATIC_CAST(const xed_uint8_t *, pemu_inst.PEMU_inst_buf), 15);

	const xed_inst_t * ins = xed_decoded_inst_inst(&pemu_inst.PEMU_xedd_g);
	const xed_operand_t *op = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name = xed_operand_name(op);
	
	if (operand_is_mem(op_name, &mem_addr, 0)) {
		PEMU_read_mem(mem_addr,sizeof(target) , &target);
	}
	else if (operand_is_reg(op_name, &reg_id)){
		target = PEMU_get_reg(reg_id);
	}
	else{
		int len = xed_decoded_inst_get_length(&pemu_inst.PEMU_xedd_g);
		target = xed_decoded_inst_get_branch_displacement(&pemu_inst.PEMU_xedd_g) + pc + len;
	}
	return target;
}



xed_error_enum_t disas_one_inst_ex(target_ulong pc, struct PEMU_INST *inst)
{
	PEMU_read_mem(pc, 15, inst->PEMU_inst_buf);
	xed_decoded_inst_zero_set_mode(&inst->PEMU_xedd_g, &inst->PEMU_dstate);
	xed_error_enum_t xed_error = xed_decode(&inst->PEMU_xedd_g,
			XED_STATIC_CAST(const xed_uint8_t *, inst->PEMU_inst_buf), 15);
	xed_decoded_inst_dump_att_format(&inst->PEMU_xedd_g, inst->PEMU_inst_str, sizeof(inst->PEMU_inst_str), 0);
	//printf("%x\t%s\n", pc, inst->PEMU_inst_str);
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

unsigned int disas_get_target(unsigned int start_pc, struct PEMU_INST *inst)
{

	PEMU_read_mem(start_pc, 15, inst->PEMU_inst_buf);
	xed_decoded_inst_zero_set_mode(&inst->PEMU_xedd_g, &inst->PEMU_dstate);
	xed_error_enum_t xed_error = xed_decode(&inst->PEMU_xedd_g,
			XED_STATIC_CAST(const xed_uint8_t *, inst->PEMU_inst_buf), 15);


	if (xed_error != XED_ERROR_NONE) {
		fprintf(stderr, "error in disas_get_target\n");
		exit(0);
	}

	const xed_inst_t *xi = xed_decoded_inst_inst(&inst->PEMU_xedd_g);
	if(xed_decoded_inst_get_iclass(&inst->PEMU_xedd_g) != XED_ICLASS_CALL_NEAR) {
		return 0;
	}

	const xed_operand_t *op = xed_inst_operand(xi, 0);
	xed_reg_enum_t reg_id;
	xed_operand_enum_t op_name = xed_operand_name(op);
	unsigned int dest, tmp;
	
	if(operand_is_relbr(op_name, &dest)){
		dest += (start_pc +  xed_decoded_inst_get_length(&inst->PEMU_xedd_g));				
	}else if(operand_is_reg(op_name, &reg_id)){
		dest = PEMU_get_reg(reg_id);	
	}else if(operand_is_mem4(op_name, &dest,0)){
		PEMU_read_mem(dest, 4, &tmp);
		dest = tmp;
	}
	return dest;
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

void helper_call_handler(target_ulong pc)
{
	//printf("%x %x\n", pc, Instrument_CALL_NEAR(pc));
	if(pc > 0xc0000000) {
		recover_sem_types(Instrument_CALL_NEAR(pc));
	}
}


void helper_call_handler1(target_ulong pc)
{
	if(pc > 0xc0000000) {
		add_call_dst(Instrument_CALL_NEAR(pc));
	}
}


extern unsigned int g_pc;
void helper_load1(target_ulong src, int size)
{
	{
		NodeType *s, *d;

		if(src < 0xc0000000 || size != 4) {
			return;
		}

		target_ulong dst;
		PEMU_read_mem(src, 4, &dst);
		if((s = ds_code_rbtFind2(src))
				&& (d = ds_code_rbtFind2(dst))) {
			add_pointTo(g_pc, s->type, d->type);
		}
	}
}
