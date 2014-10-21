#include "parse_operand.h"
#include "qemu-common.h"
#include "qemu-pemu.h"

//extern int xed_regmapping[][3];
//extern struct CPUX86State* cpu_single_env;
uint32_t s_base, s_index, s_scale, s_dis;
xed_reg_enum_t s_base_regid, s_index_regid;
extern int xed_regmapping[][3];

int operand_is_reg(const xed_operand_enum_t op_name, xed_reg_enum_t * reg_id) 
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


int operand_is_relbr(const xed_operand_enum_t op_name, uint32_t * branch) 
{
	switch (op_name) {
		/* Jumps */
	case XED_OPERAND_PTR:	// pointer (always in conjunction with a IMM0)
	case XED_OPERAND_RELBR:{
				// branch displacements

			xed_uint_t disp =
			    xed_decoded_inst_get_branch_displacement(&pemu_inst.PEMU_xedd_g);
			*branch = disp;
			 return 1;

		} default:return 0;
	}


}


int operand_is_imm(const xed_operand_enum_t op_name, uint32_t *value) {
	switch (op_name) {
		/* Immediate */
	case XED_OPERAND_IMM0:{
			if (xed_decoded_inst_get_immediate_is_signed(&pemu_inst.PEMU_xedd_g)) {
				xed_int32_t signed_imm_val =
				    xed_decoded_inst_get_signed_immediate
				    (&pemu_inst.PEMU_xedd_g);
				*value = (uint32_t) signed_imm_val;
			} else {
				xed_uint64_t unsigned_imm_val =
				    xed_decoded_inst_get_unsigned_immediate
				    (&pemu_inst.PEMU_xedd_g);
				*value = (uint32_t) unsigned_imm_val;
			}
			return 1;
		}
		/* Special immediate only used in ENTER instruction */
	case XED_OPERAND_IMM1:{
			xed_uint8_t unsigned_imm_val =
			    xed_decoded_inst_get_second_immediate(&pemu_inst.PEMU_xedd_g);
			*value = (uint32_t) unsigned_imm_val;
			return 1;
		}

	default:
		return 0;
	}

}


int get_mem_operand_size(const xed_operand_enum_t op_name, uint32_t operand_i){
	switch (op_name) {
		/* Memory */
	case XED_OPERAND_AGEN:
	case XED_OPERAND_MEM0:
	case XED_OPERAND_MEM1:
		return xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, operand_i);
	default:
		return 0;

	}
}


int operand_is_mem(const xed_operand_enum_t op_name, uint32_t* mem_addr, 
		   int operand_i)
{
	switch (op_name) {
		/* Memory */
	case XED_OPERAND_AGEN:
	case XED_OPERAND_MEM0:
	case XED_OPERAND_MEM1:{
			unsigned long base = 0;
			unsigned long index = 0;
			unsigned long scale = 1;
			unsigned long segbase = 0;
			unsigned long displacement = 0;
			unsigned long segsel = 0;
//			size_t remaining = 0;

			/* Set memory index */
			int mem_idx = 0;
			if (op_name == XED_OPERAND_MEM1)
				mem_idx = 1;

			/* Initialization */
			base = 0;
			index = 0;
			scale = 0;
			segbase = 0;
			displacement = 0;

			segbase = 0;
			// Get Base register
			xed_reg_enum_t base_regid = s_base_regid 
				= xed_decoded_inst_get_base_reg(&pemu_inst.PEMU_xedd_g, mem_idx);
			
			if (base_regid != XED_REG_INVALID) {
				base = PEMU_get_reg(base_regid);
			}
			// Get Index register and Scale
			xed_reg_enum_t index_regid = s_index_regid
			    = xed_decoded_inst_get_index_reg(&pemu_inst.PEMU_xedd_g, mem_idx);
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
			//Get Segment register
			xed_reg_enum_t seg_regid =
                xed_decoded_inst_get_seg_reg(&pemu_inst.PEMU_xedd_g, mem_idx);

			displacement =
			    (unsigned long)
			    xed_decoded_inst_get_memory_displacement(&pemu_inst.PEMU_xedd_g,
								     mem_idx);

			if (seg_regid != XED_REG_INVALID) {
                const xed_operand_values_t *xopv =
                    xed_decoded_inst_operands_const(&pemu_inst.PEMU_xedd_g);
                xed_bool_t default_segment =
                    xed_operand_values_using_default_segment
                    (xopv, mem_idx);

                if (!default_segment) {
                    int segmentreg =
                        xed_regmapping[seg_regid][0];

					struct CPUX86State* env = (struct CPUX86State*)(first_cpu->env_ptr);
                    segbase = env->segs[segmentreg].base;
                    segsel = env->segs[segmentreg].selector;
                }
              //debug(vmac_log, "SEG %d %s segbase %x, segsel %d \n", seg_regid, \
                xed_reg_enum_t2str(seg_regid), segbase, segsel);
            }

			*mem_addr =
			    segbase + base + index * scale + displacement;
			s_base = base;
			s_dis = segbase + displacement;
			//fprintf(stdout, "%x %x %x %x %x\n", segbase, base, index, scale, displacement);
			return 1;
		}

	default:
		return 0;
	}

}



uint32_t get_callDest(const xed_decoded_inst_t* xedd, uint32_t cur_pc)
{
#ifdef DEBUG
	fprintf(stdout, "data:instrument_call\n");
#endif
	const xed_inst_t* xi = xed_decoded_inst_inst(xedd);
	xed_reg_enum_t reg_id;
	uint32_t buf;
	unsigned int dest = 0;
	const xed_operand_t *op = xed_inst_operand(xi, 0);
	xed_operand_enum_t op_name = xed_operand_name(op);

	if(operand_is_mem(op_name, &dest, 0)){
		PEMU_read_mem(dest, 4, &buf);
		dest = buf;
	}else if(operand_is_reg(op_name, &reg_id)){
		dest = PEMU_get_reg(reg_id);
	}else if(operand_is_relbr(op_name, &dest)){
		dest += (cur_pc +  xed_decoded_inst_get_length(&pemu_inst.PEMU_xedd_g));
	}
	return dest;
}

