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


/*
 * *****************************************************************
 * *****************************************************************
 * *****************************************************************
 */

/******************Global Data Section***************************/


#define DEBUG_TAINT
#define ENABLE_TAINT

#include "taint.h"

static uint32_t g_pc;
static char g_inst_str[500];
static char g_inst_buffer[15];
static char g_inst_name[1024];
static xed_iclass_enum_t g_opcode;
uint32_t kernel_stack_min;
uint32_t kernel_stack_max;

static struct {
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
} syscall_args;	

#define SET_TAINT(XXX, T)	\
	t = PEMU_get_reg(XXX);	\
	if(t >= 0x8000000 && t < user_esp + 10000){	\
		printf("setting argument taint at addr: 0x%x\n", t);	\
		if(!get_mem_taint(t)){	\
			set_mem_taint_bysize(t, T, 20);	\
		}	\
	}

#define DUMP_TAINT(XXX) \
	t = XXX;	\
	while(get_mem_taint(t) != 0) {	\
		printf("memory addr: %x taint: %x\n", t, get_mem_taint(t));	\
		t++;	\
	}	\
	printf("\n");

void set_taint_source_args(void)
{
#ifdef ENABLE_TAINT
	fprintf(stdout, "start set_taint_source_args\n");
	uint32_t t;
	uint32_t user_esp = get_user_esp();
	uint32_t kernel_esp = get_kernel_esp();
	kernel_stack_min = kernel_esp & 0xffffe000;
	kernel_stack_max = kernel_stack_min + 0x2000;
	printf("user_esp:%x kernel_stack_min:%x kernel_stack_max:%x\n", 
			user_esp, kernel_stack_min, kernel_stack_max);
#ifdef FREEBSD
//TODO:
	uint32_t para1, para2, para3, para4, para5, para6;
	char tmp[100];
	PEMU_read_mem(esp + 1*4, 4, &para1);
	PEMU_read_mem(esp + 2*4, 4, &para2);
	PEMU_read_mem(esp + 3*4, 4, &para3);
	PEMU_read_mem(esp + 4*4, 4, &para4);
	PEMU_read_mem(esp + 5*4, 4, &para5);
	PEMU_read_mem(esp + 6*4, 4, &para6);

	SET_TAINT(para1, 1)
	SET_TAINT(para2, 2)
	SET_TAINT(para3, 3)
	SET_TAINT(para4, 4)
	SET_TAINT(para5, 5)
	SET_TAINT(para6, 6)

	set_mem_taint_bysize(esp + 1*4, 1, 4);
	set_mem_taint_bysize(esp + 2*4, 2, 4);
	set_mem_taint_bysize(esp + 3*4, 3, 4);
	set_mem_taint_bysize(esp + 4*4, 4, 4);
	set_mem_taint_bysize(esp + 5*4, 5, 4);
	set_mem_taint_bysize(esp + 6*4, 6, 4);

#else

	syscall_args.ebx = PEMU_get_reg(XED_REG_EBX);
	syscall_args.ecx = PEMU_get_reg(XED_REG_ECX);
	syscall_args.edx = PEMU_get_reg(XED_REG_EDX);
	syscall_args.esi = PEMU_get_reg(XED_REG_ESI);
	syscall_args.edi = PEMU_get_reg(XED_REG_EDI);
	syscall_args.ebp = PEMU_get_reg(XED_REG_EBP);
//#if 0
//	SET_TAINT(XED_REG_EBX, 1)
//	SET_TAINT(XED_REG_ECX, 2)
//	SET_TAINT(XED_REG_EDX, 3)
//	SET_TAINT(XED_REG_ESI, 4)
//	SET_TAINT(XED_REG_EDI, 5)
//	SET_TAINT(XED_REG_EBP, 6)

#if 0
	set_reg_taint(XED_REG_EBX, 1);
	set_reg_taint(XED_REG_ECX, 2);
	set_reg_taint(XED_REG_EDX, 3);
	set_reg_taint(XED_REG_ESI, 4);
	set_reg_taint(XED_REG_EDI, 5);
	set_reg_taint(XED_REG_EBP, 6);
#endif
	//printf("ebx=%x\tecx=%x\n", PEMU_get_reg(XED_REG_EBX),PEMU_get_reg(XED_REG_ECX));
	//printf("kernel_stack_min=%x\tkernel_stack_max=%x\n", kernel_stack_min, kernel_stack_max);

#endif
	fprintf(stdout, "end set_taint_source_args\n");
#endif
}


void dump_taint_return(void)
{
#if 0
	uint32_t t, i;
	printf("\ndumping taint value:\n");
	printf("eax=%x\n", get_reg_taint(XED_REG_EAX));
	DUMP_TAINT(syscall_args.ebx)
	DUMP_TAINT(syscall_args.ecx)
	DUMP_TAINT(syscall_args.edx)
	DUMP_TAINT(syscall_args.esi)
	DUMP_TAINT(syscall_args.edi)
	DUMP_TAINT(syscall_args.ebp)
#endif
}


static inline int is_special_inst(const xed_decoded_inst_t *x)
{
	switch(xed_decoded_inst_get_iclass(x))
	{
		case XED_ICLASS_LEA:
			return 1;
		default:
			return 0;	
	}
}

#if 0
static unsigned int handle_control_target(INS xi)
{
	const xed_operand_t *op = xed_inst_operand(xi, 0);
	xed_reg_enum_t reg_id;
	xed_operand_enum_t op_name = xed_operand_name(op);
	unsigned int dest = 0;

	if(operand_is_reg(op_name, &reg_id)){
		dest = PEMU_get_reg(reg_id);
	}
	else if(operand_is_mem(op_name, &dest, 0)){
		PEMU_read_mem(dest, 4, &dest);
	}
	else if(operand_is_relbr(op_name, &dest)){
		dest += (g_pc +  xed_decoded_inst_get_length(&pemu_inst.PEMU_xedd_g));
	}else{
		fprintf(stderr, "error: Instrument_CALL\n");
		exit(0);
	}
	return dest;
}
#endif

static void get_enviroment_taint(INS ins)
{
	uint32_t addr;
	const xed_operand_t *op_1 = xed_inst_operand(ins, 1);
	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_1 = xed_operand_name(op_1);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	if(is_special_inst(&pemu_inst.PEMU_xedd_g)) {
		return;
	}

	switch(g_opcode) {
		case XED_ICLASS_MOVSB://special cases that the memory accesses can't be detected
		case XED_ICLASS_MOVSW:
		case XED_ICLASS_MOVSD:
			if(!PEMU_get_reg(XED_REG_ECX) 
				&& xed_operand_values_has_rep_prefix(
					xed_decoded_inst_operands_const(&pemu_inst.PEMU_xedd_g))
				)
				return;
			addr = PEMU_get_reg(XED_REG_ESI);
			break;
		case XED_ICLASS_PUSH:
			if(!operand_is_mem(op_name_0, &addr, 0))
				return;
			break;
		case XED_ICLASS_PUSHFD:
		case XED_ICLASS_PUSHAD:
		case XED_ICLASS_POP:
		case XED_ICLASS_POPFD:
		case XED_ICLASS_POPAD:
			return;
		default:
			if(operand_is_mem(op_name_1, &addr, 1)) {
				//printf("Here\n");
				break;
			}
			else return;
	}
	
	if(addr > 0xc0000000 && (addr < kernel_stack_min || addr > kernel_stack_max)) {
//		if(get_mem_taint(addr)) { //already have pc as taint value
//			return;
//		}
//		fprintf(stdout, "taint source value: %x function: %x addr: %x\n", 
//				g_pc, get_current_func(), addr);
//		set_mem_taint_bysize(addr, g_pc, 
//				xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 1));
		fprintf(stderr, "get environment: %x function: %x addr: %x\n", 
				g_pc, get_current_func(), addr);

	}
}

static void set_enviroment_taint(INS ins)
{
	uint32_t addr;
	const xed_operand_t *op_1 = xed_inst_operand(ins, 1);
	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_1 = xed_operand_name(op_1);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	if(is_special_inst(&pemu_inst.PEMU_xedd_g)) {
		return;
	}

	switch(g_opcode) {
		case XED_ICLASS_MOVSB://special cases that the memory accesses can't be detected
		case XED_ICLASS_MOVSW:
		case XED_ICLASS_MOVSD:
			if(!PEMU_get_reg(XED_REG_ECX) 
				&& xed_operand_values_has_rep_prefix(
					xed_decoded_inst_operands_const(&pemu_inst.PEMU_xedd_g))
				)
				return;
			addr = PEMU_get_reg(XED_REG_EDI);
			break;
		case XED_ICLASS_PUSH:
		case XED_ICLASS_PUSHFD:
		case XED_ICLASS_PUSHAD:
			return;
		case XED_ICLASS_POP:
			if (!operand_is_mem(op_name_0, &addr, 0))
				return;
			else break;
		case XED_ICLASS_POPFD:
		case XED_ICLASS_POPAD:
			return;
		default:
			if(!operand_is_mem(op_name_1, &addr, 1))
				return;
			else break;
	}
	
	if(addr > 0xc0000000 && (addr < kernel_stack_min || addr > kernel_stack_max)) {
		fprintf(stderr, "set environment: %x function: %x addr: %x\n", 
				g_pc, get_current_func(), addr);
		//dump_callstacks_ex(g_pc);
	}
}

//#if 0
static void ds_code_handle_mem_access1(INS ins)
{
	uint32_t addr, value;
	const xed_operand_t *op_1 = xed_inst_operand(ins, 1);
	xed_operand_enum_t op_name_1 = xed_operand_name(op_1);

	if(is_special_inst(&pemu_inst.PEMU_xedd_g)) {
		return;
	}

	if(operand_is_mem(op_name_1, &addr, 1)) {
		fprintf(stdout, "access memory address: %x\ttaint=%x\n", addr, get_mem_taint(addr));
	}
	if(addr > 0xc0000000 && (addr < kernel_stack_min || addr > kernel_stack_max)
			&& get_mem_taint(addr) == 4
			&& g_pc < 0xe0000000) {
		dump_callstacks_ex(g_pc);
	}
}

static void ds_code_handle_mem_access0(INS ins)
{
	uint32_t addr;
	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	if(operand_is_mem(op_name_0, &addr, 0)) {
		fprintf(stdout, "access memory address: %x\ttaint=%x\n", addr, get_mem_taint(addr));
	}
	if(addr > 0xc0000000 && (addr < kernel_stack_min || addr > kernel_stack_max)
			&& get_mem_taint(addr) == 4
			&& g_pc < 0xe0000000) {
		dump_callstacks_ex(g_pc);
	}
}
//#endif

static void taint_mem_access0(INS ins)
{
	uint32_t addr;
	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	if(operand_is_mem(op_name_0, &addr, 0)){
		int size = xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 0);
//#if 0
		if(addr < kernel_stack_min  || addr > kernel_stack_max) {
			fprintf(stderr, "pc\t%x\tpropagate to heap mem:\tpara_%d\n", g_pc, get_mem_taint(addr));
		}
//#endif

	}
}


static InstrumentFunction instrument_functions[XED_ICLASS_LAST];

static unsigned int g_taint;

static void UnimplementedInstruction(INS ins) 
{
	fprintf(stderr, "missing:\t%s\n", xed_iclass_enum_t2str(g_opcode));
	return;
}

static void Instrument_LEA(INS xi)
{
	
	xed_reg_enum_t reg_id;
	uint32_t value;
	uint32_t mem_addr;
	UChar taint = 0;


	const xed_operand_t *op = xed_inst_operand(xi, 0);
	xed_operand_enum_t op_name = xed_operand_name(op);

	if (operand_is_reg(op_name, &reg_id)) {

		op = xed_inst_operand(xi, 1);
		op_name = xed_operand_name(op);

		if (operand_is_mem(op_name, &mem_addr, 1) && reg_id != XED_REG_ESP) 
		{
			taint = get_reg_taint(s_base_regid);
			set_reg_taint(reg_id, taint);
		}
	}
}

static void Instrument_CALL(INS xi)
{
}

static void Instrument_RET(INS xi)
{
}


static void Instrument_NONE(INS ins)
{
}




static void Instrument_LODSX(INS ins)
{
	if(!PEMU_get_reg(XED_REG_ECX) && xed_operand_values_has_rep_prefix(xed_decoded_inst_operands_const(&pemu_inst.PEMU_xedd_g)))
		return;

	const xed_operand_t *op_1 = xed_inst_operand(ins, 1);
	xed_operand_enum_t op_name_1 = xed_operand_name(op_1);

	uint32_t addr;
	if(operand_is_mem(op_name_1, &addr, 1)){
		g_taint = get_mem_taint(addr);
		set_reg_taint(XED_REG_EAX, g_taint);
	}

#ifdef DEBUG_TAINT
//	if(g_taint){
//		fprintf(stdout, "taint at pc\t%x\n", g_pc);
//	}
#endif
}



static void Instrument_STOSX(INS ins)
{
	if(!PEMU_get_reg(XED_REG_ECX) && xed_operand_values_has_rep_prefix(xed_decoded_inst_operands_const(&pemu_inst.PEMU_xedd_g)))
		return;

	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	uint32_t addr;
	if(operand_is_mem(op_name_0, &addr, 0)){
		g_taint = get_reg_taint(XED_REG_EAX);
		set_mem_taint_bysize(addr, g_taint, xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 0));
	}

#ifdef DEBUG_TAINT
//	if(g_taint){
//		fprintf(stdout, "taint at pc\t%x\n", g_pc);
//	}
#endif
}


static void Instrument_MOVSBWD(INS ins) 
{
	if(!PEMU_get_reg(XED_REG_ECX) && xed_operand_values_has_rep_prefix(xed_decoded_inst_operands_const(&pemu_inst.PEMU_xedd_g)))
		return;

	g_taint = get_mem_taint(PEMU_get_reg(XED_REG_ESI));

	set_mem_taint_bysize(PEMU_get_reg(XED_REG_EDI), g_taint, xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 0));

#ifdef DEBUG_TAINT
//	if(g_taint){
//		fprintf(stdout, "taint at pc1\t%x\n", g_pc);
//	}
	printf("MOVS %x (%x) -> %x (%x)\n", 
			PEMU_get_reg(XED_REG_ESI), get_mem_taint(PEMU_get_reg(XED_REG_ESI)), 
			PEMU_get_reg(XED_REG_EDI), get_mem_taint(PEMU_get_reg(XED_REG_EDI)));
#endif

}


static void Instrument_MOV(INS ins)
{
	xed_reg_enum_t reg;
	uint32_t size, addr;
	
	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	const xed_operand_t *op_1 = xed_inst_operand(ins, 1);
	xed_operand_enum_t op_name_1 = xed_operand_name(op_1);

	NodeType *tmp;
	g_taint = 0;
	if(operand_is_mem(op_name_1, &addr, 1)){
		g_taint = get_mem_taint(addr);
	}else if(operand_is_reg(op_name_1, &reg)){
		g_taint = get_reg_taint(reg);
	}else{
		g_taint = 0;
	}

	if(operand_is_mem(op_name_0, &addr, 0)){
		size = xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 0);
		set_mem_taint_bysize(addr, g_taint, size);
	}else if(operand_is_reg(op_name_0, &reg)){
		set_reg_taint(reg, g_taint);
	}

#ifdef DEBUG_TAINT
//	if(g_taint){
//		fprintf(stdout, "taint at pc\t%x\n", g_pc);
//	}
#endif
}

static void Instrument_CMOVcc(INS ins) 
{
	xed_iclass_enum_t opcode = xed_decoded_inst_get_iclass(&pemu_inst.PEMU_xedd_g);

	int cont = 0;
	struct CPUX86State* cpu_single_env = (struct CPUX86State*)(first_cpu->env_ptr);
	uint32_t eflags = cpu_compute_eflags(cpu_single_env);

	//printf("eflags:\t%x\t%x\n", cpu_single_env->eflags, eflags);

	switch(opcode){
		case XED_ICLASS_CMOVB:
			cont = is_cf_set(eflags);
			break;
		case XED_ICLASS_CMOVNB:
			cont = !is_cf_set(eflags);
			break;
		case XED_ICLASS_CMOVZ:
			cont = is_zf_set(eflags);
			break;
		case XED_ICLASS_CMOVNZ:
			cont = !is_zf_set(eflags);
			break;
		case XED_ICLASS_CMOVBE:
			cont = is_zf_set(eflags) || is_cf_set(eflags);
			break;
		case XED_ICLASS_CMOVNBE:
			cont = !is_zf_set(eflags) && !is_cf_set(eflags);
			break;
		case XED_ICLASS_CMOVS:
			cont = is_sf_set(eflags);
			break;
		case XED_ICLASS_CMOVNS:
			cont = !is_sf_set(eflags);
			break;
		case XED_ICLASS_CMOVL:
			cont = is_sf_set(eflags) != is_of_set(eflags);
			//cont = ((eflags & CC_S ? 1 : 0) != (eflags & CC_O) ? 1 : 0);
			break;
		case XED_ICLASS_CMOVNL:
			cont = is_sf_set(eflags) == is_of_set(eflags);
			break;
		case XED_ICLASS_CMOVLE:
			cont = is_sf_set(eflags) != is_of_set(eflags) || is_zf_set(eflags);
			break;
		case XED_ICLASS_CMOVNLE:
			cont = is_sf_set(eflags) == is_of_set(eflags) && !is_zf_set(eflags);
			break;
		default:
			printf("unfound CMOVcc!\n");
			exit(0);
	}

	if(cont)
		Instrument_MOV(ins);

}


static void Instrument_SETcc(INS ins)
{
	const xed_operand_t *op = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name = xed_operand_name(op);
	xed_iclass_enum_t opcode = xed_decoded_inst_get_iclass(&pemu_inst.PEMU_xedd_g);

	uint32_t addr;
	xed_reg_enum_t reg;

	if(operand_is_mem(op_name, &addr, 0)){
		set_mem_taint_bysize(addr, 0, 1);
	}else if(operand_is_reg(op_name, &reg)){
		set_reg_taint(reg, 0);
	}


}



static void Instrument_Binary_OP(INS ins)//TODO: if parameter bop local_variable ==> local_variable
{
	xed_reg_enum_t reg;
	uint32_t size, addr, value;
	
	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	const xed_operand_t *op_1 = xed_inst_operand(ins, 1);
	xed_operand_enum_t op_name_1 = xed_operand_name(op_1);

	g_taint = 0;
	if(operand_is_mem(op_name_1, &addr, 1)){
		g_taint = get_mem_taint(addr);
	}else if(operand_is_reg(op_name_1, &reg)){
		g_taint = get_reg_taint(reg);
	}else if(operand_is_imm(op_name_1, &value)){
		goto END;
	}

	if(operand_is_mem(op_name_0, &addr, 0)){
		size = xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 0);
		g_taint |= get_mem_taint(addr);
		set_mem_taint_bysize(addr, g_taint, size);
	}else if(operand_is_reg(op_name_0, &reg)){
		g_taint |= get_reg_taint(reg);
		set_reg_taint(reg, g_taint);
	}

#ifdef DEBUG_TAINT
END:
//	if(g_taint){
//		fprintf(stdout, "taint at pc\t%x\n", g_pc);
//	}
	return;
#endif
}



static void Instrument_XADD(INS ins)//TODO: if parameter bop local_variable ==> local_variable
{
	
	xed_reg_enum_t reg0, reg1;
	uint32_t addr0, addr1;
	
	const xed_operand_t *op_0 = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name_0 = xed_operand_name(op_0);

	const xed_operand_t *op_1 = xed_inst_operand(ins, 1);
	xed_operand_enum_t op_name_1 = xed_operand_name(op_1);

	uint32_t taint0 = 0, taint1 = 0;
	if(operand_is_mem(op_name_0, &addr0, 0)){
		taint0 = get_mem_taint(addr0);
		if(operand_is_reg(op_name_1, &reg1)){
			taint1 = get_reg_taint(reg1);
			set_mem_taint_bysize(addr0, taint1, xed_decoded_inst_operand_length(&pemu_inst.PEMU_xedd_g, 0));
			set_reg_taint(reg1, taint0);
		}
	}else if(operand_is_reg(op_name_0, &reg0)){
		taint0 = get_reg_taint(reg0);
		if(operand_is_reg(op_name_1, &reg1)){
			taint1 = get_reg_taint(reg1);
			set_reg_taint(reg1, taint0);
			set_reg_taint(reg0, taint1);
		}
	}

	Instrument_Binary_OP(ins);

}




static void Instrument_PUSH(INS ins) 
{	
	xed_reg_enum_t reg_id;
	uint32_t mem_addr;
	const xed_operand_t *op = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name = xed_operand_name(op);
			
	uint32_t value;
	uint32_t addr;

    mem_addr = PEMU_get_reg(XED_REG_ESP) - 4;
	g_taint = 0;
	if (operand_is_reg(op_name, &reg_id)) {
		g_taint = get_reg_taint(reg_id);
	}else if(operand_is_imm(op_name, &value)) {
		g_taint = 0;
	}else if(operand_is_mem(op_name, &addr, 0)) {
		g_taint = get_mem_taint(addr);
	}else {
		#ifdef DEBUG_TAINT
		fprintf(stderr, "%d unknown\n", 1);
		#endif
		return;
	}

	set_mem_taint_bysize(mem_addr, g_taint, 4);

#ifdef DEBUG_TAINT
	fprintf(stdout, "push addr=%x\ttaint = %x\n", 
			mem_addr, get_mem_taint(mem_addr));
//	if(g_taint)
//		fprintf(stdout, "taint at pc\t%x\n", g_pc);
#endif	
}


static void Instrument_PUSHFD(INS ins)
{

	target_ulong mem_addr = PEMU_get_reg(XED_REG_ESP) - 4;
	set_mem_taint_bysize(mem_addr, 0, 4);

}

static void Instrument_POPFD(INS ins)
{

}


static void Instrument_PUSHAD(INS ins) {

	unsigned int regs[] = {XED_REG_EAX, XED_REG_ECX, XED_REG_EDX, XED_REG_EBX,
		XED_REG_ESP, XED_REG_EBP, XED_REG_ESI, XED_REG_EDI};

	struct CPUX86State* cpu_single_env = (struct CPUX86State*)(first_cpu->env_ptr);
	target_ulong mem_addr = cpu_single_env->regs[R_ESP] - 4;
	
	unsigned int i=0;
	for(i = 0; i < 8; i++, mem_addr = mem_addr - 4){
		set_mem_taint(mem_addr, get_reg_taint(regs[i]));
	}
}



static void Instrument_POPAD(INS ins){ 
	unsigned int regs[] = {XED_REG_EDI, XED_REG_ESI, XED_REG_EBP, XED_REG_ESP,
		XED_REG_EBX, XED_REG_EDX, XED_REG_ECX, XED_REG_EAX};
	
	target_ulong mem_addr = PEMU_get_reg(XED_REG_ESP);
	
	unsigned int i=0;
	for(i = 0; i < 8; i++, mem_addr = mem_addr + 4){
		set_reg_taint(regs[i], get_mem_taint(mem_addr));
		set_mem_taint_bysize(mem_addr, 0, 4);
	}
}


static void Instrument_POP(INS ins)
{	
	xed_reg_enum_t reg_id;
	uint32_t mem_addr, addr;	

	mem_addr = PEMU_get_reg(XED_REG_ESP);
	g_taint = get_mem_taint(mem_addr);

	const xed_operand_t *op = xed_inst_operand(ins, 0);
	xed_operand_enum_t op_name = xed_operand_name(op);
	
	if(operand_is_reg(op_name, &reg_id)){
		set_reg_taint(reg_id, g_taint);		
	}else if (operand_is_mem(op_name, &addr, 0)){
		set_mem_taint_bysize(mem_addr, g_taint, 4);
	}

#ifdef DEBUG_TAINT
	fprintf(stdout, "pop addr=%x\n", mem_addr);
//	if(g_taint)
//		fprintf(stdout, "taint at pc\t%x\n", g_pc);
#endif	
}

static void Instrument_XOR(INS xi) 
{
	xed_reg_enum_t reg_id;
	uint32_t value;
	target_ulong mem_addr;


	const xed_operand_t *op = xed_inst_operand(xi, 0);
	xed_operand_enum_t op_name = xed_operand_name(op);

	if (operand_is_reg(op_name, &reg_id)) {
		op = xed_inst_operand(xi, 1);
		op_name = xed_operand_name(op);

		xed_reg_enum_t reg_id2;

		if (operand_is_reg(op_name, &reg_id2))
		{
			if(reg_id == reg_id2)
			{
				set_reg_taint(reg_id, 0);
				return;
			}
		}
	}
	Instrument_Binary_OP(xi);
}


static void Instrument_CMP(INS xi) //special
{
}

void setup_inst_hook(void)
{
	int i;
	for (i = 0; i < XED_ICLASS_LAST; i++) {
		instrument_functions[i] = &UnimplementedInstruction;
	}
	
	taintInit();
	//taintInit2();

	instrument_functions[XED_ICLASS_LEA] = &Instrument_LEA;
	instrument_functions[XED_ICLASS_CALL_NEAR] = &Instrument_CALL;
	instrument_functions[XED_ICLASS_RET_NEAR] = &Instrument_RET;
	instrument_functions[XED_ICLASS_MOV] = &Instrument_MOV;	//61

//////////////////////////////////////////////////////////////////
	instrument_functions[XED_ICLASS_JO] = &Instrument_NONE;	//42
	instrument_functions[XED_ICLASS_JNO] = &Instrument_NONE;	//43
	instrument_functions[XED_ICLASS_JB] = &Instrument_NONE;	//43
	instrument_functions[XED_ICLASS_JNB] = &Instrument_NONE;	//45
	instrument_functions[XED_ICLASS_JZ] = &Instrument_NONE;	//46
	instrument_functions[XED_ICLASS_JNZ] = &Instrument_NONE;	//47
	instrument_functions[XED_ICLASS_JBE] = &Instrument_NONE;	//48
	instrument_functions[XED_ICLASS_JNBE] = &Instrument_NONE;	//49
	instrument_functions[XED_ICLASS_JS] = &Instrument_NONE;	//50
	instrument_functions[XED_ICLASS_JNS] = &Instrument_NONE;	//51
	instrument_functions[XED_ICLASS_JP] = &Instrument_NONE;	//52
	instrument_functions[XED_ICLASS_JNP] = &Instrument_NONE;	//53
	instrument_functions[XED_ICLASS_JL] = &Instrument_NONE;	//54
	instrument_functions[XED_ICLASS_JNL] = &Instrument_NONE;	//55
	instrument_functions[XED_ICLASS_JLE] = &Instrument_NONE;	//56
	instrument_functions[XED_ICLASS_JNLE] = &Instrument_NONE;	//57
	instrument_functions[XED_ICLASS_JRCXZ] = &Instrument_NONE;	//57
	instrument_functions[XED_ICLASS_JMP] = &Instrument_NONE;

	instrument_functions[XED_ICLASS_CLD] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_CLI] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_STI] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_MFENCE] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_SYSEXIT] = &Instrument_NONE;


	instrument_functions[XED_ICLASS_CMP] = &Instrument_CMP;
	instrument_functions[XED_ICLASS_CMPXCHG] = &Instrument_NONE;	//TODO
	instrument_functions[XED_ICLASS_TEST] = &Instrument_NONE;

	instrument_functions[XED_ICLASS_DEC] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_INC] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_NEG] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_NOT] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_BTS] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_BT] = &Instrument_NONE;

	instrument_functions[XED_ICLASS_CBW] = &Instrument_NONE;	//67
	instrument_functions[XED_ICLASS_CWDE] = &Instrument_NONE;	//67
	instrument_functions[XED_ICLASS_CDQ] = &Instrument_NONE;	//70

	instrument_functions[XED_ICLASS_RDTSC] = &Instrument_NONE;//TODO
	instrument_functions[XED_ICLASS_IRETD] = &Instrument_NONE;//TODO
	instrument_functions[XED_ICLASS_PREFETCHNTA] = &Instrument_NONE;//TODO
	instrument_functions[XED_ICLASS_LFENCE] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_MOV_CR] = &Instrument_NONE;//TODO


	instrument_functions[XED_ICLASS_SCASB] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_SCASD] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_BSR] = &Instrument_NONE;//TODO
	instrument_functions[XED_ICLASS_BSF] = &Instrument_NONE;//TODO
	instrument_functions[XED_ICLASS_BTS] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_BTR] = &Instrument_NONE;	

	instrument_functions[XED_ICLASS_OUT] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_IN] = &Instrument_NONE;

	instrument_functions[XED_ICLASS_NOP] = &Instrument_NONE;

/////////////////////////////////////////////////////////////////////

	instrument_functions[XED_ICLASS_LODSB] = &Instrument_LODSX; //93
	instrument_functions[XED_ICLASS_LODSD] = &Instrument_LODSX; //95
	instrument_functions[XED_ICLASS_LODSD] = &Instrument_LODSX; //95

	instrument_functions[XED_ICLASS_STOSB] = &Instrument_STOSX;	//89
	instrument_functions[XED_ICLASS_STOSW] = &Instrument_STOSX; //90
	instrument_functions[XED_ICLASS_STOSD] = &Instrument_STOSX;	//91

	instrument_functions[XED_ICLASS_XOR] = &Instrument_XOR;	// 15
	instrument_functions[XED_ICLASS_OR] = &Instrument_Binary_OP;	// 15
	instrument_functions[XED_ICLASS_AND] = &Instrument_Binary_OP;	//61
	instrument_functions[XED_ICLASS_ADD] = &Instrument_Binary_OP;	//61
	instrument_functions[XED_ICLASS_ADC] = &Instrument_Binary_OP;
	instrument_functions[XED_ICLASS_SUB] = &Instrument_Binary_OP;
	instrument_functions[XED_ICLASS_SBB] = &Instrument_Binary_OP;

	instrument_functions[XED_ICLASS_XADD] = &Instrument_XADD;


	instrument_functions[XED_ICLASS_MOVSB] = &Instrument_MOVSBWD;	//81
	instrument_functions[XED_ICLASS_MOVSW] = &Instrument_MOVSBWD;	//82
	instrument_functions[XED_ICLASS_MOVSD] = &Instrument_MOVSBWD;	//83
	
	instrument_functions[XED_ICLASS_CMOVB] = &Instrument_CMOVcc;	//177
	instrument_functions[XED_ICLASS_CMOVNB] = &Instrument_CMOVcc;	//178
	instrument_functions[XED_ICLASS_CMOVZ] = &Instrument_CMOVcc;	//179
	instrument_functions[XED_ICLASS_CMOVNZ] = &Instrument_CMOVcc;	//180
	instrument_functions[XED_ICLASS_CMOVBE] = &Instrument_CMOVcc;	//181
	instrument_functions[XED_ICLASS_CMOVNBE] = &Instrument_CMOVcc;	//182
	instrument_functions[XED_ICLASS_CMOVS] = &Instrument_CMOVcc;	//307
	instrument_functions[XED_ICLASS_CMOVNS] = &Instrument_CMOVcc;	//308
	instrument_functions[XED_ICLASS_CMOVL] = &Instrument_CMOVcc;	//311
	instrument_functions[XED_ICLASS_CMOVNL] = &Instrument_CMOVcc;	//312
	instrument_functions[XED_ICLASS_CMOVLE] = &Instrument_CMOVcc;	//313
	instrument_functions[XED_ICLASS_CMOVNLE] = &Instrument_CMOVcc;	//314




	instrument_functions[XED_ICLASS_SETB] = &Instrument_SETcc;	//222
	instrument_functions[XED_ICLASS_SETNB] = &Instrument_SETcc;	//223
	instrument_functions[XED_ICLASS_SETZ] = &Instrument_SETcc;	//224
	instrument_functions[XED_ICLASS_SETNZ] = &Instrument_SETcc;	//225
	instrument_functions[XED_ICLASS_SETBE] = &Instrument_SETcc;	//226
	instrument_functions[XED_ICLASS_SETNBE] = &Instrument_SETcc;	//227


	instrument_functions[XED_ICLASS_MOVZX] = &Instrument_MOV;	
	instrument_functions[XED_ICLASS_MOVSX] = &Instrument_MOV;	

	instrument_functions[XED_ICLASS_PUSH] = &Instrument_PUSH;
	instrument_functions[XED_ICLASS_PUSHFD] = &Instrument_PUSHFD;	//74
	instrument_functions[XED_ICLASS_PUSHAD] = &Instrument_PUSHAD;	//74
	instrument_functions[XED_ICLASS_POP] = &Instrument_POP;
	instrument_functions[XED_ICLASS_POPFD] = &Instrument_POPFD;	//77
	instrument_functions[XED_ICLASS_POPAD] = &Instrument_POPAD;	//77


	instrument_functions[XED_ICLASS_SHL] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_SHR] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_SAR] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_RCL] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_RCR] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_ROL] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_ROR] = &Instrument_NONE;
//	instrument_functions[XED_ICLASS_SAL] = &Instrument_NONE;
	instrument_functions[XED_ICLASS_SHRD] = &Instrument_NONE;//TODO
	instrument_functions[XED_ICLASS_SHLD] = &Instrument_NONE;//TODO

	instrument_functions[XED_ICLASS_DIV] = &Instrument_Binary_OP;	//482
	instrument_functions[XED_ICLASS_IDIV] = &Instrument_Binary_OP;	//483
	instrument_functions[XED_ICLASS_MUL] = &Instrument_Binary_OP;
	instrument_functions[XED_ICLASS_IMUL] = &Instrument_Binary_OP;
}


void Instrument(uint32_t pc, INS ins)
{
	xed_iclass_enum_t opcode = g_opcode = xed_decoded_inst_get_iclass(&pemu_inst.PEMU_xedd_g);

	xed_decoded_inst_dump_att_format(&pemu_inst.PEMU_xedd_g, g_inst_str, sizeof(g_inst_str), 0);
	g_pc = pc;
	printf("%x:\t%s\n", g_pc, g_inst_str);

#ifdef ENABLE_TAINT
	get_enviroment_taint(ins); //gettor taint source
#if 0
	//taint propagation:
	g_taint = 0;
	instrument_functions[opcode](ins);
	if(0 < g_taint && g_taint <= 6) {
		printf("taint value:%d appears\n", g_taint);
		taint_mem_access0(ins);
	}
#endif
	set_enviroment_taint(ins);//creator: only one
#endif
//	ds_code_handle_mem_access1(ins);
//	ds_code_handle_mem_access0(ins);
}
