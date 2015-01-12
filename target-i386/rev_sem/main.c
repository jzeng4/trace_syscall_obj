#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pemu.h"
#include "lib_map.h"
#include "parse_operand.h"
#include "config_pemu.h"
#include "qemu/host-utils.h"
#include "cpu.h"
#include "tcg-op.h"
#include "disas.h"
#include "heap_shadow.h"

#include "helper.h"
#define GEN_HELPER 1
#include "helper.h"


uint32_t enable_trace;
uint32_t g_pc;
extern int s_in_sched;
extern int s_int_level;
extern int s_in_syscall;
extern int s_trace;
extern int s_esp;
extern int s_sysnum;
extern FILE *output_file;
extern FILE *output_database;
extern int start_cpuid;

#define in_syscall_context()	\
	s_in_syscall == 1


//#define DEBUG

size_t hash_callstack(unsigned int);
unsigned int get_cur_callstack(void);

void add_obj(uint32_t addr, uint32_t size, char *name, uint32_t call_pc)
{
	NodeType hp;
	hp.size = size;
	hp.type = hash_callstack(call_pc);
	hp.pc = call_pc;
	strcpy(hp.obj_name, name);
	delete_obj(call_pc);
	ds_code_all_insert_rb(addr, hp.size, hp);

	add_instance(hp.type, addr);
}

void object_hook(int pc_start)
{
	uint32_t objsize, ret_addr, type, addr;
	char name[50];
#if 0
	//for vmalloc
	if(pc_start == 0xc10b81b7) {
		printf("in 0xc10b81b7\n");
		save_vmalloc(hash_callstack(pc_start));
	}
	return;
#endif

	if(get_obj(pc_start)) {
		get_name(pc_start, name);
		//uint32_t obj_addr = get_obj_addr(pc_start) != 0 ? get_obj_addr(pc_start) : PEMU_get_reg(XED_REG_EAX);
		uint32_t obj_addr = PEMU_get_reg(XED_REG_EAX);


		//TODO	
		//if(strcmp(name, "task_struct")) {
		//	return;
		//}
		//instance_action_print(obj_addr, 0, g_pc, 0);
		//add_instance(obj_addr);
		//end
		
		add_obj(obj_addr, get_size(pc_start), name, pc_start);
	}
	
	if(pc_start == KMEM_CACHE_ALLOC) {
		
		//TODO:
		//if(cur_get_ret() != 0xc11cd7c8) {
		//	return;
		//}
		//end
			
		uint32_t tmp, tmp1;
		if(is_dup_call_kmem_cache_alloc()) {//some special cases
			return;
		}

		PEMU_read_mem(PEMU_get_reg(XED_REG_ESP), 4, &ret_addr);
		get_kmem_cache_alloc_args(&objsize, name);
		insert_obj(ret_addr, 0, objsize, name);
	} else if(pc_start == TRACE_KMALLOC) {
		//TODO
		//return;
		//end
		uint32_t addr, size;
		get_trace_kmalloc_args(&addr, &size);
		PEMU_read_mem(PEMU_get_reg(XED_REG_ESP), 4, &ret_addr);
#ifdef LINUX_2_6_32_8_NO_TRACE //in case kmem_cache_alloc and ktrace are together
		if(disas_is_call(ret_addr-15)) {
			return;
		}
#endif
		insert_obj(ret_addr, addr, size, "size-XX");
	} else if(pc_start == KMEM_CACHE_FREE) {
		get_kmem_cache_free_args(&addr);		
		ds_code_all_delete_rb(addr);
	} else if(pc_start == KFREE){
		uint32_t addr1 = PEMU_get_reg(XED_REG_EAX);
		ds_code_all_delete_rb(addr1);
	}
}


void trace_delete(unsigned int pc_start)
{
	unsigned int addr;
	NodeType *p;
	if(pc_start == KMEM_CACHE_FREE) {
		get_kmem_cache_free_args(&addr);

		if((p = ds_code_rbtFind2(addr)) != NULL) {
			//instance_action_print(addr, 0, g_pc, 3);
			delete_instance(addr);
		}
		ds_code_all_delete_rb(addr);
			
	} else if(pc_start == KFREE){
		addr = PEMU_get_reg(XED_REG_EAX);
		if((p = ds_code_rbtFind2(addr)) != NULL) {
			//instance_action_print(addr, 0, g_pc, 3);
			delete_instance(addr);
		}
		ds_code_all_delete_rb(addr);
	}

}

void trace_functions(int pc_start)
{
	unsigned int target;
	if((target = disas_get_target(pc_start, &pemu_inst))) {
		//set_syscall_call_funcs(target);
		set_call_relation(get_parent_callstack(), target);
	}
}


void callstack_hook(int pc_start)
{
	if(is_cur_retaddr(pc_start)) {
		cur_delete_callstack();
		if(cur_delete_retaddr()) {
			syscall_exit(get_kernel_esp() & 0xffffe000, 3);			
		}
		//cur_dump_rets(stdout, 0);
	}
	if(disas_callnear_ex(pc_start, &pemu_inst) == XED_ERROR_NONE) {
		if(xed_decoded_inst_get_iclass(&pemu_inst.PEMU_xedd_g) == XED_ICLASS_CALL_NEAR) {
			uint32_t retaddr = pc_start + xed_decoded_inst_get_length(&pemu_inst.PEMU_xedd_g);
			cur_insert_retaddr(retaddr);
			cur_insert_callstack(get_callDest(&pemu_inst.PEMU_xedd_g, pc_start));
			//cur_dump_rets(stdout, 0);
		}
	}
}

#if 0
void helper_store(target_ulong value, target_ulong addr, int size)
{
	if(enable_trace) 
	{
		NodeType *tmp;

		if(addr < 0xc0000000 || (addr >= s_esp && addr < s_esp + STACK_SIZE) || s_esp == 0) {
			return;
		}
		
		//in case access kernel objects in ____cache_alloc
		if(g_pc >= ____CACHE_ALLOC_START && g_pc <= ____CACHE_ALLOC_END) {
			return;
		}

		if(tmp = ds_code_rbtFind2(addr)) {
			set_writeSys(s_sysnum, addr, tmp);
#ifdef DEBUG
			fprintf(stdout, "Sys:%x Write: addr:%x type:%llx name:%s ret:%x cr3:%x esp:%x\n", 
				s_sysnum, addr, tmp->type, tmp->obj_name, tmp->ret, tmp->cr3, tmp->esp);
#endif
		}
	}
}


void helper_load(target_ulong addr, int size)
{
	if(enable_trace) 
	{
		NodeType *tmp;
		
		if(addr < 0xc0000000 || (addr >= s_esp && addr < s_esp + STACK_SIZE) || s_esp == 0) {
			return;
		}
		
		//in case access kernel objects in ____cache_alloc
		if(g_pc >= ____CACHE_ALLOC_START && g_pc <= ____CACHE_ALLOC_END) {
			return;
		}

		if(tmp = ds_code_rbtFind2(addr)) {
			set_readSys(s_sysnum, addr, tmp);
#ifdef DEBUG
			printf("key:%x size:%x addr:%x\n", tmp->key, tmp->size, addr);
			fprintf(stdout, "Sys:%x Read: addr:%x type:%llx name:%s ret:%x cr3:%x esp:%x\n", 
				s_sysnum, addr, tmp->type, tmp->obj_name, tmp->ret, tmp->cr3, tmp->esp);
#endif
		}
	}
}
#endif

void helper_store(target_ulong value, target_ulong addr, int size)
{
	{
		NodeType *tmp;

		if(addr < 0xc0000000 || (addr >= s_esp && addr < s_esp + STACK_SIZE)) {
			return;
		}
		
		//in case access kernel objects in ____cache_alloc
		if(g_pc >= ____CACHE_ALLOC_START && g_pc <= ____CACHE_ALLOC_END) {
			return;
		}

		if(tmp = ds_code_rbtFind2(addr)) {
			add_action(tmp->key, addr-tmp->key, g_pc, 2);
		}
	}
}


void helper_load(target_ulong addr, int size)
{
	{
		NodeType *tmp;
		
		if(addr < 0xc0000000 || (addr >= s_esp && addr < s_esp + STACK_SIZE)) {
			return;
		}
		
		//in case access kernel objects in ____cache_alloc
		if(g_pc >= ____CACHE_ALLOC_START && g_pc <= ____CACHE_ALLOC_END) {
			return;
		}

		if(tmp = ds_code_rbtFind2(addr)) {
			add_action(tmp->key, addr-tmp->key, g_pc, 1);
		}
	}
}

void trace_kmem_create(int pc_start)
{
#if 0 //for linux
	static uint32_t size = 0;	
	if(get_kmem_obj(pc_start)){
		char name[100];
		get_kmem_name(pc_start, name);
		fprintf(stderr, "%x %s %d\n", 
				PEMU_get_reg(XED_REG_EAX), name, size);
		delete_kmem(pc_start);
	}

	//if(pc_start == KMEM_CACHE_CREATE){
	if(pc_start == 0xc10c302f) {
		uint32_t ret_addr;
		char name[100];
		size = PEMU_get_reg(XED_REG_EDX);
		PEMU_read_mem(PEMU_get_reg(XED_REG_EAX), 100, name);
		PEMU_read_mem(PEMU_get_reg(XED_REG_ESP), 4, &ret_addr);
		insert_kmem_obj(ret_addr, name);
	}
#endif
	//freebsd
	static uint32_t size = 0;	
	if(get_kmem_obj(pc_start)){
		char name[100];
		get_kmem_name(pc_start, name);
		fprintf(stderr, "%x %s %d\n", 
				PEMU_get_reg(XED_REG_EAX), name, size);
		delete_kmem(pc_start);
	}

	//if(pc_start == KMEM_CACHE_CREATE){
	if(pc_start == 0xc0d22a60) {
		uint32_t ret_addr;
		uint32_t ptr;
		char name[100];
		PEMU_read_mem(PEMU_get_reg(XED_REG_ESP), 4, &ret_addr);
		PEMU_read_mem(PEMU_get_reg(XED_REG_ESP) + 4, 4, &ptr);
		PEMU_read_mem(PEMU_get_reg(XED_REG_ESP) + 8, 4, &size);
		PEMU_read_mem(ptr, 20, name);
		insert_kmem_obj(ret_addr, name);
	}
}

void find_kstack_switch(int pc_start)
{
	if(pc_start > 0xc0000000)
		if(disas_one_inst_ex(pc_start, &pemu_inst) == XED_ERROR_NONE) {
			uint32_t addr;
			xed_reg_enum_t reg_id_0, reg_id_1;
			const xed_operand_t *op_0 = xed_inst_operand(
					xed_decoded_inst_inst(&pemu_inst.PEMU_xedd_g), 0);
			const xed_operand_t *op_1 = xed_inst_operand(
					xed_decoded_inst_inst(&pemu_inst.PEMU_xedd_g), 1);
			xed_operand_enum_t op_name_0 = xed_operand_name(op_0);
			xed_operand_enum_t op_name_1 = xed_operand_name(op_1);
			int type = xed_decoded_inst_get_iclass(&pemu_inst.PEMU_xedd_g);
			
			if(type == XED_ICLASS_PUSH || type == XED_ICLASS_LEA) return;
			if(type == XED_ICLASS_POP) {
				if (!operand_is_reg(op_name_0, &reg_id_0)) {
					if(reg_id_0 == XED_REG_ESP)
						fprintf(stderr, "pop esp\n");
				}
			}
			
			if(operand_is_reg(op_name_0, &reg_id_0)) {
				if(reg_id_0 == XED_REG_ESP) {
					if(operand_is_reg(op_name_1, &reg_id_1)) {
						addr = PEMU_get_reg(reg_id_1);
					} else if(operand_is_mem(op_name_1, &addr, 1)) {
						PEMU_read_mem(addr, 4, &addr);
					} else {
						return;
					}
					if(PEMU_get_reg(XED_REG_ESP) < 0xc0000000) {
						return;
					}
					if((PEMU_get_reg(XED_REG_ESP) & 0xffffe000) != 
							(addr & 0xffffe000)) {
						fprintf(stderr, "pc:%x src=%x dst=%x\n", pc_start, 
								addr, PEMU_get_reg(XED_REG_ESP));
					}
				}
			} 
		}

	return;
}

void trace_tss_esp(uint32_t pc_start)
{
	if(pc_start == 0xc1003138) {
		printf("esp:%x\n", PEMU_get_reg(XED_REG_ESP));
	}
}


#if 0
void trace_all_activity(uint32_t pc_start) 
{
	uint32_t objsize, ret_addr, type, addr;
	char name[50];
	
	if(get_obj1(pc_start)) {
		ds_code_erase_all(PEMU_get_reg(XED_REG_EAX), get_size1(pc_start));
		delete_obj1(pc_start);
	}
	
	if(pc_start == KMEM_CACHE_ALLOC) {
		uint32_t tmp, tmp1;
		PEMU_read_mem(PEMU_get_reg(XED_REG_ESP), 4, &ret_addr);
		get_kmem_cache_alloc_args(&objsize, name);
		//insert_obj1(ret_addr, objsize);
		return;
		//printf("KMEM_CACHE_ALLOC %x\n", get_obj1(ret_addr));
	} else if(pc_start == KMEM_CACHE_FREE) {
		get_kmem_cache_free_args(&addr);
		ds_code_all_delete_rb(addr);
	} else if(pc_start == TRACE_KMALLOC) {
		uint32_t addr, size;
		get_trace_kmalloc_args(&addr, &size);
		PEMU_read_mem(PEMU_get_reg(XED_REG_ESP), 4, &ret_addr);
		//ds_code_erase_all(addr, size);
	} else if(pc_start == KFREE) {
		ds_code_all_delete_rb(PEMU_get_reg(XED_REG_EAX));
		ds_code_all_delete_rb(PEMU_get_reg(XED_REG_EBX));
	}
}

#endif

void helper_hook(int pc_start)
{

#if 0
	if(pc_start == KMEM_CACHE_ALLOC) {
		uint32_t tmp;
		char name[100];
		get_kmem_cache_alloc_args(&tmp, name);
		record_name(name);
	}
	return;
#endif

#if 0
	if(pc_start < 0xc0000000) 
		return;
	trace_kmem_create(pc_start);
	return;
#endif

#if 0
	find_kstack_switch(pc_start);
	return;
#endif

#if 0
	trace_tss_esp(pc_start);
	return;
#endif

#if 0
	if(pc_start == TRACE_KMALLOC) {
		uint32_t esp = PEMU_get_reg(XED_REG_ESP);
		uint32_t para1=0, para2=0;
		PEMU_read_mem(esp+0x4, 4, &para1);
		PEMU_read_mem(esp+0x8, 4, &para2);
		printf("eax:%x  esi:%x ecx:%d para1:%d para2:%d\n", 
				PEMU_get_reg(XED_REG_EAX), PEMU_get_reg(XED_REG_ESI), PEMU_get_reg(XED_REG_ECX),
				para1, para2);
	}
	return;
#endif

#ifdef TRACE_SYSCALL_OBJ

	enable_trace = 0;
	
	if(pc_start < KERNEL_ADDR_MIN)
		return;
	
	g_pc = pc_start;

	if(pc_start == ESP_SWITCH_PONIT) {//switch esp
		new_proc_start(pc_start);
	}

	//catch all the deletions:
	//trace_all_activity(pc_start);	
	trace_delete(pc_start);
	//end

	if(in_syscall_context()) {
		if(pc_start == SCHED_ADDR) {
			uint32_t ret_addr;
			PEMU_read_mem(PEMU_get_reg(XED_REG_ESP), 4, &ret_addr);
			s_in_sched = ret_addr;
			//printf("esp:%x inSchedule\n", get_kernel_esp() & 0xffffe000);
		}
			
		if(pc_start == s_in_sched) {
			s_in_sched = 0;
			//printf("esp:%x outSchedule\n", get_kernel_esp() & 0xffffe000);
		}

		if(s_in_sched == 0 && s_int_level == 0) {
#ifdef CALLSTACK
			callstack_hook(pc_start);
#endif

#if 0
#ifdef MYCPUID
			if(start_cpuid == 0) {
				return;
			}
			if(get_kernel_esp() == pemu_exec_stats.PEMU_kern_esp)
				trace_functions(pc_start);
			return;

#endif
#endif

			object_hook(pc_start);

#ifdef INFER_SIZE
			infer_size(pc_start);
#endif

#if 0
#ifdef MYCPUID
	if(start_cpuid == 0) {
		return;
	}
#endif
#endif
			enable_trace = 1;
		}
	}
#endif
}


