#include "linux.h"

/*
TEMU is Copyright (C) 2006-2009, BitBlaze Team.

TEMU is based on QEMU, a whole-system emulator. You can redistribute
and modify it under the terms of the GNU LGPL, version 2.1 or later,
but it is made available WITHOUT ANY WARRANTY. See the top-level
README file for more details.

For more information about TEMU and other BitBlaze software, see our
web site at: http://bitblaze.cs.berkeley.edu/
*/

//#include "config.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "config_pemu.h"
#include "qemu-pemu.h"
#include "pemu.h"
#include "linux.h"


#if 0
/* need to check next_task_struct with the corresponding 
   kernel source code for compatibality 
   in 2.4.20 the next pointer points directly to the next
   tast_struct, while in 2.6.15, it is done through list_head */
struct koffset kernel_table[] = {
	{
	"2.6.38-8-generic",               /* entry name */
   	0xC1060460, 0x00000000,      /* hooking address: flush_signal_handlers */
	0xC1731F60, /* task struct root */
	3228, /* size of task_struct */
	432, /* offset of task_struct list */
	508, /* offset of pid */
	460, /* offset of mm */
	40, /* offset of pgd in mm */
	732, /* offset of comm */
	16, /* size of comm */
	4, /* offset of vm_start in vma */
	8, /* offset of vm_end in vma */
	12, /* offset of vm_next in vma */
	76, /* offset of vm_file in vma */
	24, /*offset of vm_flags in vma*/
	12, /* offset of dentry in file */
	20, /* offset of d_name in dentry */
	36, /* offset of d_iname in dentry */
	0xc111ac10, /*address of kmem_cache_alloc*/
	0xc111a540,  /*address of kmem_cache_free*/
	0x0	/*kernel esp*/
	},
	
	{
	"2.6.32.8",               /* entry name */
   	0xc103913a, 0x00000000,      /* hooking address: flush_signal_handlers */
	0xC136eba0, /* task struct root */
	1072, /* size of task_struct */
	228, /* offset of task_struct list */
	288, /* offset of pid */
	256, /* offset of mm */
	36, /* offset of pgd in mm */
	536, /* offset of comm */
	16, /* size of comm */
	4, /* offset of vm_start in vma */
	8, /* offset of vm_end in vma */
	12, /* offset of vm_next in vma */
	72, /* offset of vm_file in vma */
	0, /*offset of vm_flags in vma*/
	12, /* offset of dentry in file */
	32, /* offset of d_name in dentry */
	92, /* offset of d_iname in dentry */
	0xc10a9a1b, /*address of kmem_cache_alloc*/
	0xc10a997f, /*address of kmem_cache_free*/
	0xc1c07f80-0x217c  /*kernel esp*/
	},

	{
	"3.2.58",               /* entry name */
   	0xc103913a, 0x00000000,      /* hooking address: flush_signal_handlers */
	0xC136eba0, /* task struct root */
	1072, /* size of task_struct */
	228, /* offset of task_struct list */
	288, /* offset of pid */
	256, /* offset of mm */
	36, /* offset of pgd in mm */
	536, /* offset of comm */
	16, /* size of comm */
	4, /* offset of vm_start in vma */
	8, /* offset of vm_end in vma */
	12, /* offset of vm_next in vma */
	72, /* offset of vm_file in vma */
	0, /*offset of vm_flags in vma*/
	12, /* offset of dentry in file */
	32, /* offset of d_name in dentry */
	92, /* offset of d_iname in dentry */
	0xc10a9a1b, /*address of kmem_cache_alloc*/
	0xc10a997f, /*address of kmem_cache_free*/
	0xdfbf2800-0x217c  /*kernel esp*/
	},



  {"", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

struct PEMU_guest_os pemu_guest_os;
uint32_t get_kernel_esp(void);

//extern "C"{
int init_kernel_offsets(void)
{
  int retval = -1;
  int i = 2;

  assert(0);
    pemu_guest_os.hookingpoint = kernel_table[i].hookingpoint;
    pemu_guest_os.hookingpoint2 = kernel_table[i].hookingpoint2;
    pemu_guest_os.taskaddr = kernel_table[i].taskaddr;
    pemu_guest_os.tasksize = kernel_table[i].tasksize;
    pemu_guest_os.listoffset = kernel_table[i].listoffset;
    pemu_guest_os.pidoffset = kernel_table[i].pidoffset;
    pemu_guest_os.mmoffset = kernel_table[i].mmoffset;
    pemu_guest_os.pgdoffset = kernel_table[i].pgdoffset;
    pemu_guest_os.commoffset = kernel_table[i].commoffset;
    pemu_guest_os.commsize = kernel_table[i].commsize;
    pemu_guest_os.vmstartoffset = kernel_table[i].vmstartoffset;
    pemu_guest_os.vmendoffset = kernel_table[i].vmendoffset;
    pemu_guest_os.vmnextoffset = kernel_table[i].vmnextoffset;
    pemu_guest_os.vmfileoffset = kernel_table[i].vmfileoffset;
	pemu_guest_os.vmflagsoffset = kernel_table[i].vmflagsoffset;
    pemu_guest_os.dentryoffset = kernel_table[i].dentryoffset;
    pemu_guest_os.dnameoffset = kernel_table[i].dnameoffset;
    pemu_guest_os.dinameoffset = kernel_table[i].dinameoffset;

	pemu_guest_os.kmem_cache_alloc_addr = kernel_table[i].kmem_cache_alloc_addr;
	pemu_guest_os.kmem_cache_free_addr = kernel_table[i].kmem_cache_free_addr;
	pemu_guest_os.sysenter_esp = kernel_table[i].sysenter_esp;
  	retval = 0;

	return retval;
}


///////////////////////////
static void get_name(uint32_t addr, int size, char *buf)
{
   PEMU_read_mem(addr + pemu_guest_os.commoffset, 16, buf);
}

static uint32_t next_task_struct(uint32_t addr)
{
	uint32_t retval;
	uint32_t next;

    PEMU_read_mem(addr + pemu_guest_os.listoffset + sizeof(uint32_t), 
			sizeof(uint32_t), &next);
    retval = next - pemu_guest_os.listoffset;

  	return retval;
}

static uint32_t get_pid(uint32_t addr)
{
  	uint32_t pid;

  	PEMU_read_mem(addr + pemu_guest_os.pidoffset, sizeof(pid), &pid);
	return pid;
}

static uint32_t get_pgd(uint32_t addr)
{
	uint32_t mmaddr, pgd;
	PEMU_read_mem(addr + pemu_guest_os.mmoffset, sizeof(mmaddr), &mmaddr);
	
	if (0 == mmaddr)
		PEMU_read_mem(addr + pemu_guest_os.mmoffset + sizeof(mmaddr), 
  				sizeof(mmaddr), &mmaddr);

	if (0 != mmaddr)
	   	PEMU_read_mem(mmaddr + pemu_guest_os.pgdoffset, sizeof(pgd), &pgd);
	else
	   	memset(&pgd, 0, sizeof(pgd));

	return pgd;
}

static uint32_t get_first_mmap(uint32_t addr)
{
	uint32_t mmaddr, mmap;
	PEMU_read_mem(addr + pemu_guest_os.mmoffset, sizeof(mmaddr), &mmaddr);

	if (0 == mmaddr)
		PEMU_read_mem(addr + pemu_guest_os.mmoffset + sizeof(mmaddr), 
                   sizeof(mmaddr), &mmaddr);

  	if (0 != mmaddr)
	 	PEMU_read_mem(mmaddr, sizeof(mmap), &mmap);
	else
		memset(&mmap, 0, sizeof(mmap));
	
	return mmap;
}

static void get_mod_name(uint32_t addr, char *name, int size)
{
	uint32_t vmfile, dentry;

	if(PEMU_read_mem(addr + pemu_guest_os.vmfileoffset, sizeof(vmfile), &vmfile) != 0
			|| PEMU_read_mem(vmfile + pemu_guest_os.dentryoffset, sizeof(dentry), &dentry) != 0
			|| PEMU_read_mem(dentry + pemu_guest_os.dinameoffset, size < 36 ? size : 36, name) != 0)
		name[0] = 0;
}

static uint32_t get_vmstart(uint32_t addr)
{
	uint32_t vmstart;
	PEMU_read_mem(addr + pemu_guest_os.vmstartoffset, sizeof(vmstart), &vmstart);
  	return vmstart;
}

static uint32_t get_next_mmap(uint32_t addr)
{
  	uint32_t mmap;
	PEMU_read_mem(addr + pemu_guest_os.vmnextoffset, sizeof(mmap), &mmap);
	return mmap;
}

static uint32_t get_vmend(uint32_t addr)
{
   	uint32_t vmend;
	PEMU_read_mem(addr + pemu_guest_os.vmendoffset, sizeof(vmend), &vmend);
	return vmend;
}

static uint32_t get_vmflags(uint32_t addr)
{
	uint32_t vmflags;
	PEMU_read_mem(addr + pemu_guest_os.vmflagsoffset, sizeof(vmflags), &vmflags);
	return vmflags;
}
/////////////////////////////

int PEMU_find_process(void *opaque)
{
	uint32_t nextaddr = 0;
	char comm[512];
	int count = 0;
	struct CPUX86State *env = (struct CPUX86State*) opaque;

	nextaddr = pemu_guest_os.taskaddr;
	do{
		if (++count > 1000)
			return -1;
	  	get_name(nextaddr, 16, comm);
		
		if(!strcmp(pemu_exec_stats.PEMU_binary_name, "all")
				&& !strncmp(comm, "TR_", 3))
			break;
		if(!strcmp(comm, pemu_exec_stats.PEMU_binary_name))
			break;
		nextaddr = next_task_struct(nextaddr);
	}while(nextaddr != pemu_guest_os.taskaddr);

	if(nextaddr != pemu_guest_os.taskaddr){
		pemu_exec_stats.PEMU_pid = get_pid(nextaddr);
		pemu_exec_stats.PEMU_cr3 = get_pgd(nextaddr) - 0xc0000000;
		pemu_exec_stats.PEMU_task_addr = nextaddr;
		pemu_exec_stats.PEMU_kern_esp = get_kernel_esp(); 
		fprintf(stdout, "finding process %s %x %x %x\n", 
				comm, pemu_exec_stats.PEMU_pid, pemu_exec_stats.PEMU_cr3, 
				pemu_exec_stats.PEMU_kern_esp);
		return 1;
	}
	return 0;
}

struct Module_info {
	char name[64];
	uint32_t init_func;
	uint32_t module_init;
	uint32_t module_core;
	uint32_t init_size, core_size;
	uint32_t init_text_size, core_text_size;

};

int PEMU_find_module(void *opaque)
{
//	if(pc == 0xc1088907)//hook sys_init_module
	{
		struct Module_info PEMU_module;
		memset(&PEMU_module, 0, sizeof(struct Module_info));

		uint32_t mod = PEMU_get_reg(XED_REG_EAX);
		PEMU_read_mem(mod + 0xc, sizeof(PEMU_module.name), PEMU_module.name);
		PEMU_read_mem(mod + 0xd4, sizeof(PEMU_module.init_func), &PEMU_module.init_func);
		PEMU_read_mem(mod + 0xd8, sizeof(PEMU_module.module_init), &PEMU_module.module_init);
		PEMU_read_mem(mod + 0xdc, sizeof(PEMU_module.module_core), &PEMU_module.module_core);
		PEMU_read_mem(mod + 0xe0, sizeof(PEMU_module.init_size), &PEMU_module.init_size);
		PEMU_read_mem(mod + 0xe4, sizeof(PEMU_module.core_size), &PEMU_module.core_size);
		PEMU_read_mem(mod + 0xe8, sizeof(PEMU_module.init_text_size), &PEMU_module.init_text_size);
		PEMU_read_mem(mod + 0xec, sizeof(PEMU_module.core_text_size), &PEMU_module.core_text_size);
		
		fprintf(stderr, "new module insert:\t%s\tinit_func:%x\tmodule_init:%x\tmodule_core:%x\tinit_size:%x\tcore_size:%x \
				init_text_size:%x\tcore_text_size:%x\n", PEMU_module.name, PEMU_module.init_func, PEMU_module.module_init, 
				PEMU_module.module_core, PEMU_module.init_size, PEMU_module.core_size, PEMU_module.init_text_size, 
				PEMU_module.core_text_size);
	}
	return 1;
}

#endif



uint32_t get_kernel_esp(void)
{
	uint32_t esp;
#ifdef FREEBSD_9_1
	uint32_t ss;
	struct CPUX86State* env=(struct CPUX86State*)(first_cpu->env_ptr);
	PEMU_read_mem(env->tr.base+4, 4, &esp);
#else
	PEMU_read_mem(KERNEL_ESP, 4, &esp);
#endif
	return esp;
}

#if 0
uint32_t get_user_esp(void)
{
	pemu_guest_os.user_esp = PEMU_get_reg(XED_REG_ESP);
	return pemu_guest_os.user_esp;
}
#endif

void get_kmem_cache_alloc_args(uint32_t *objsize, char *name)
{
	uint32_t tmp, tmp1;

#ifdef FREEBSD_9_1
	PEMU_read_mem(PEMU_get_reg(XED_REG_ESP) + 4, 4, &tmp);
	PEMU_read_mem(tmp, 4, &tmp1);
	PEMU_read_mem(tmp1, 50, name);
	PEMU_read_mem(tmp+4*16, 4, &tmp1);
	*objsize = tmp1;
	
	//cur_dump_callstack(stdout, 0);
	//printf("name: %s\n", name);
#endif

#ifdef LINUX_2_6_32_8
	tmp = PEMU_get_reg(XED_REG_EAX);
	PEMU_read_mem(tmp + 0x8, 4, objsize);
	PEMU_read_mem(tmp + 0x54, 4, &tmp1);
	PEMU_read_mem(tmp1, 50, name);
#endif

#ifdef LINUX_2_6_32_8_NO_TRACE
	tmp = PEMU_get_reg(XED_REG_EAX);
	PEMU_read_mem(tmp + 0x4, 4, objsize);
	PEMU_read_mem(tmp + 0x54, 4, &tmp1);
	PEMU_read_mem(tmp1, 50, name);
#endif

#ifdef LINUX_3_2_58
	assert(0);
#endif

#ifdef LINUX_3_2_58_NO_TRACE
	tmp = PEMU_get_reg(XED_REG_EAX);
	PEMU_read_mem(tmp + 12, 4, objsize);
	PEMU_read_mem(tmp + 0x3c, 4, &tmp1);
	PEMU_read_mem(tmp1, 50, name);
#endif

#ifdef INFER_SIZE
	*objsize = 1;
#endif

	//printf("kmem_cache_alloc:%s %d\n", name, *objsize);
}

void get_kmem_cache_free_args(uint32_t *addr)
{
	uint32_t tmp;
#ifdef FREEBSD_9_1
	PEMU_read_mem(PEMU_get_reg(XED_REG_ESP) + 8, 4, addr);
#else
	*addr = PEMU_get_reg(XED_REG_EDX);
#endif
}


void get_trace_kmalloc_args(uint32_t *addr, uint32_t *size)
{
	uint32_t tmp;
#ifdef FREEBSD_9_1
	assert(0);
#endif

#ifdef LINUX_2_6_32_8
	assert(0);
#endif

#ifdef LINUX_2_6_32_8_NO_TRACE
	*addr = PEMU_get_reg(XED_REG_EAX);
	*size = PEMU_get_reg(XED_REG_EDX);
#endif

#ifdef LINUX_3_2_58
	assert(0);
#endif

#ifdef LINUX_3_2_58_NO_TRACE
	*addr = PEMU_get_reg(XED_REG_ESI);
	*size = PEMU_get_reg(XED_REG_ECX);
#endif

#ifdef INFER_SIZE
	*size = 1;
#endif
	//printf("trace_kmalloc:%x %x\n", *addr, *size);
}

void get_kcreate_args(uint32_t *objsize, char *name)
{
	
}


extern FILE *output_file;
FILE *PEMU_open(int , int);
int start_cpuid = 0;
void init_syscall_trace(uint32_t num, int index)
{
#if 0
	pemu_exec_stats.PEMU_start = 1;
	pemu_exec_stats.PEMU_cr3 = PEMU_get_cr3();
	pemu_exec_stats.PEMU_main_start = 1;
	pemu_exec_stats.PEMU_syscall_num = num;
	pemu_exec_stats.PEMU_in_schedule = 0;
	output_file = PEMU_open(num, index);
	fprintf(stderr, "PEMU_start:%x cr3:%x main_start:%x syscall_num:%d\n", 
			pemu_exec_stats.PEMU_start, pemu_exec_stats.PEMU_cr3, pemu_exec_stats.PEMU_main_start, pemu_exec_stats.PEMU_syscall_num);
#endif
	pemu_exec_stats.PEMU_kern_esp = get_kernel_esp();
	start_cpuid = 1;
}

void uninit_syscall_trace(void)
{
#if 0
	pemu_exec_stats.PEMU_main_start = 0;
	pemu_exec_stats.PEMU_start_trace_syscall = 0;
	pemu_exec_stats.PEMU_cr3 = 0;
	PEMU_close(output_file);
#endif
	start_cpuid = 0;
	print_call_relation();
//	clear_calldata();
}
