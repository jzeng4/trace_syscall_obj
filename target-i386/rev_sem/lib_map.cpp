#include "config_pemu.h"
#include <list>
#include <stack>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <map>
#include <string>
#include "linux.h"
#include <iostream>
#include <ext/functional>
#include <sstream>
#include <set>
#include <unordered_set>
#include <assert.h>
#include "heap_shadow.h"
#include "algorithm"

extern "C" {
#include "taint.h"

struct PEMU_INST {
	unsigned int PEMU_inst_pc;
	xed_state_t PEMU_dstate;
	xed_decoded_inst_t PEMU_xedd_g;
	char PEMU_inst_buf[15];
	char PEMU_inst_str[128];
};

extern struct PEMU_INST pemu_inst;
xed_error_enum_t disas_one_inst_ex(unsigned int pc, struct PEMU_INST *inst);
int operand_is_mem(const xed_operand_enum_t op_name, uint32_t* mem_addr, int); 
int operand_is_reg(const xed_operand_enum_t op_name, xed_reg_enum_t * reg_id);
uint32_t PEMU_get_reg(xed_reg_enum_t reg_id);
int PEMU_read_mem(uint32_t vaddr, int len, void *buf);
uint32_t get_kernel_esp(void);
uint32_t PEMU_get_cr3(void);
}
using namespace std;

struct Elem {
	unsigned int addr;
	size_t type;
	int objsize;
	unsigned int pc;
};

struct Elem1 {
	string name;
	int size;
	unsigned int caller_pc;
	unsigned int addr;
};

//static list<unsigned int> g_callsite;
//static list<unsigned int> g_callstack;
static list<unsigned int> g_ret_stack;
static map<unsigned int, struct Elem1> g_obj;
static map<unsigned int, struct Elem1> g_obj1;
static map<unsigned int, int> g_all_obj;
static map<unsigned int, size_t> g_obj_pool;
static map<unsigned int, string> g_heap_types;
static map<string, unsigned int> g_heap_types_s;
static map<unsigned int, unsigned int> g_come_from;
static map<unsigned int, string> g_syscall;
static map<unsigned int, int> g_proc_intr;
static map<unsigned, list<unsigned int> > g_all_callstacks;
static map<unsigned, list<unsigned int> > g_all_retstacks;
static map<unsigned int, string> g_trace_kmem;
static map<size_t, string > read_hash_callstack;
static map<size_t, string > write_hash_callstack;
static map<size_t, string > delete_hash_callstack;

static map<unsigned int, int> syscall_call_funcs;

static map<unsigned int, struct info> g_all_info;
static list<unsigned int> *p_cur_retstack;
static list<unsigned int> *p_cur_callstack;
int s_int_level = -1;
int s_in_sched = -1;
int s_in_syscall = -1;
int s_trace = -1;
int s_esp = 0;
int s_sysnum = -1;

#define in_syscall_context()	\
	s_in_syscall == 1


extern "C"{
FILE *output_file;
void dump_rets(FILE *file, unsigned pc);
void dump_all_rets(unsigned int esp, unsigned pc);
////////////////////////////////
size_t hash_callstack(uint32_t pc)
{
//	dump_rets(stdout, pc);
#ifdef CALLSTACK
	hash<std::string> hash_fn;
	string str;
	stringstream ss;

	for(list<unsigned int>:: iterator it = (*p_cur_retstack).begin(); 
			it != (*p_cur_retstack).end(); it++) {
		ss << *it;
	}
	ss << pc;
	str = ss.str();
	return hash_fn(str);
#else
	return pc;
#endif
}

size_t hash_read_callstack(uint32_t pc)
{
	hash<std::string> hash_fn;
	string str;
	stringstream ss;
	size_t hash;

	for(list<unsigned int>:: iterator it = (*p_cur_retstack).begin(); 
			it != (*p_cur_retstack).end(); it++) {
		ss << hex <<*it;
		ss << "->";
	}
	ss << pc;
	str = ss.str();
	hash = hash_callstack(pc);
	read_hash_callstack[hash] = str;
	return hash;
}

size_t hash_write_callstack(uint32_t pc)
{
	hash<std::string> hash_fn;
	string str;
	stringstream ss;
	size_t hash;

	for(list<unsigned int>:: iterator it = (*p_cur_retstack).begin(); 
			it != (*p_cur_retstack).end(); it++) {
		ss << hex <<*it;
		ss << "->";
	}
	ss << pc;
	str = ss.str();
	hash = hash_callstack(pc);
	write_hash_callstack[hash] = str;
	return hash;
}

size_t hash_delete_callstack(uint32_t pc)
{
	hash<std::string> hash_fn;
	string str;
	stringstream ss;
	size_t hash;

	for(list<unsigned int>:: iterator it = (*p_cur_retstack).begin(); 
			it != (*p_cur_retstack).end(); it++) {
		ss << hex <<*it;
		ss << "->";
	}
	ss << pc;
	str = ss.str();
	hash = hash_callstack(pc);
	delete_hash_callstack[hash] = str;
	return hash;
}

uint32_t is_dup_call_kmem_cache_alloc(void)
{
	for (list<unsigned int>::reverse_iterator rit = (*p_cur_callstack).rbegin();
			rit != (*p_cur_callstack).rend(); ++rit) {
		if((*rit == KMEM_CACHE_ALLOC || *rit == __KMALLOC || *rit == __KMALLOC_TRACK_CALLER)
				&& rit != (*p_cur_callstack).rbegin()) {
			//in these cases, use trace_kmalloc
			return 1;
		}
		if(*rit == KMEM_CACHE_ALLOC_TRACE && rit != (*p_cur_callstack).rbegin()) {//recursive call
			return 1;
		}
	}
	return 0;
}

void insert_obj1(unsigned int pc, int size)
{
	struct Elem1 elem;
	elem.size = size;
	g_obj1[pc] = elem;
}

int get_obj1(unsigned int pc)
{
	return g_obj1.count(pc);
}

int get_size1(unsigned int pc)
{
	return g_obj1[pc].size;
}

void delete_obj1(unsigned int pc)
{
	g_obj1.erase(pc);
}

////////////////////////////////
void insert_obj(unsigned int pc, unsigned int addr, int size, char *name)
{
	struct Elem1 elem;
	elem.addr = addr;
	elem.size = size;
	elem.name = name;
	g_obj[pc] = elem;
}

int get_obj(unsigned int pc)
{
	return g_obj.count(pc);
}

void get_name(unsigned int pc, char *name)
{
	strcpy(name, g_obj[pc].name.c_str());
}

int get_size(unsigned int pc)
{
	return g_obj[pc].size;
}

int get_obj_addr(unsigned int pc)
{
	return g_obj[pc].addr;
}

unsigned int get_caller_pc(unsigned int pc)
{
	return g_obj[pc].caller_pc;
}

void delete_obj(unsigned int pc)
{
	g_obj.erase(pc);
}

void insert_all_obj(unsigned int pc)
{
	if(g_all_obj.count(pc) == 0) {
		g_all_obj[pc] = 1;
	} else {
		if(g_all_obj[pc] <= 0) {
			printf("error:insert_all_obj %x\n", g_all_obj[pc]);
			assert(0);
		}
		g_all_obj[pc] ++;
	}
}

int get_all_obj(unsigned int pc)
{
	return g_all_obj.count(pc);
}

void delete_all_obj(unsigned int pc)
{
	if(g_all_obj[pc] == 1) {
		g_all_obj.erase(pc);
	} else {
		if(g_all_obj[pc] <= 0) {
			printf("error:delete_all_obj %x\n", g_all_obj[pc]);
			assert(0);
		}
		g_all_obj[pc]--;
	}
}
////////////////////////////////

//static map<unsigned int, string> g_trace_kmem;
void insert_kmem_obj(unsigned int pc, char *name)
{
	if(g_trace_kmem.count(pc) == 0) {
		g_trace_kmem[pc] = string(name);
	} else {
	}
}

int get_kmem_obj(unsigned int pc)
{
	return g_trace_kmem.count(pc);
}

void get_kmem_name(unsigned int pc, char *name)
{
	strcpy(name, g_trace_kmem[pc].c_str());
	return;
}

void delete_kmem(unsigned int pc)
{
	g_trace_kmem.erase(pc);
}


void insert_retaddr(unsigned int pc)
{
	g_ret_stack.push_back(pc);
}

void delete_retaddr(unsigned int pc)
{
	g_ret_stack.pop_back();
}


int is_retaddr(unsigned int pc)
{
	if(g_ret_stack.empty())
		return 0;
	if(g_ret_stack.back() == pc){
		//g_ret_stack.pop();
		return 1;
	}else {
		return 0;
	}
}

void dump_rets1(FILE *file, unsigned int pc)
{
//#ifdef DEBUG
	fprintf(file, "rets:");
	for(std::list<unsigned int>::iterator it = (*p_cur_retstack).begin(); 
			it != (*p_cur_retstack).end(); ++it){
		fprintf(file, "%x->", *it);
	}
	fprintf(file, "%x\n", pc);
//#endif
}


void clear_calldata(void)
{
	//g_callsite.clear();
	//g_callstack.clear();
	
	while(!g_ret_stack.empty())
    	g_ret_stack.clear();
}


void ds_code_load_heapTypes(void)
{
#if 0

#ifdef LINUX_2_6_32_8
	FILE *file = fopen("kmem_cache_linux.log", "r");
#endif
#ifdef FREEBSD
	FILE *file = fopen("kmem_cache_freebsd.log", "r");
#endif

	if(!file){
		fprintf(stderr, "error:\tcan't find kmem_cache_linux.log\n");
		return;
		exit(0);
	}


	char type[50];
	int num;
#ifdef FREEBSD
	char line[500];
	while(fgets(line, 500, file)){
		strcpy(type, strtok(line, "\t"));
		sscanf(strtok(NULL, "\t"), "%x", &num);
		if(g_heap_types.count(num-1)){
			//fprintf(stderr, "duplicate\t%x\n", type);
		}
		g_heap_types[num-1] = string(type);
		g_heap_types_s[type] = num-1;
		fprintf(stdout, "load\t%s\t%x\n", type, num);
	}
#else
	while(fscanf(file, "%s\t%x\n", type, &num) != EOF){
		if(g_heap_types.count(num-1)){
			fprintf(stderr, "duplicate\t%x\n", type);
		}
		g_heap_types[num-1] = type;
		g_heap_types_s[type] = num-1;
		fprintf(stdout, "load\t%s\t%x\n", type, num);
	}

#endif

	for(map<string, unsigned int>::iterator it = g_heap_types_s.begin();
			it != g_heap_types_s.end(); it++){
		//fprintf(stderr, "print\t%s\t%x\n", it->first.c_str(), it->second);
	}
#endif
}

#if 0
void insert_reference(unsigned int parent, unsigned child) {
	if(parent == child) {
		printf("Circle?\n");
		return;
	}
	printf("parent:%x child:%x\n", parent, child);
	g_come_from[child] = parent;
}

void taint_parent(unsigned int addr, unsigned taint) {
	for (map<unsigned int, unsigned int>::iterator it = g_come_from.begin();
			it != g_come_from.end(); it++) {
		printf("parent:%x child:%x\n", it->second, it->first);
	}
	while (g_come_from.count(addr) > 0) {
		printf("taint addr:%x taint:%x\n", addr, taint);
		set_mem_taint_bysize2(addr, taint, 4);
		addr = g_come_from[addr];
	}
	printf("taint addr:%x taint:%x\n", addr, taint);
	set_mem_taint_bysize2(addr, taint, 4);
	printf("finish taint_parent\n");
}
#endif

FILE *PEMU_open(int syscall, int index)
{
	char path[100];
	FILE *file;
	
	sprintf(path, "%sT_%d_%s_%x", "linux-obj/", syscall, g_syscall[syscall].c_str(), index);
	file = fopen(path, "w");
	if(file == 0) {
		assert(0);
	}
	fprintf(stderr, "PEMU_open:%s file:%p\n", path, file);
	fprintf(file, "cr3=%x esp=%x\n", PEMU_get_cr3(), get_kernel_esp() & 0xffffe000);
	return file;
}

FILE *PEMU_close(FILE *f)
{
	fprintf(stderr, "PEMU_close file:%p\n", f);
	fclose(f);
}


void load_syscalls(void)
{
	FILE *file = fopen("syscalls", "r");
	char name[50];
	int num;
	if(file == NULL){
		fprintf(stderr, "error in open syscalls\n");
		exit(0);
	}

	while(fscanf(file, "%d\t%s\n", 
				&num, name) != EOF) {
		g_syscall[num] = name;
		//cout<<num<<" "<<name<<endl;
	}
}

const char *get_syscallName(int num)
{
	return g_syscall[num].c_str();
}

//#define DEBUG

struct info {
	int in_syscall;	//at syscall contex?
	int int_level;	//intr level
	int in_sched;	//schedule?
	int trace;		//trace?
	uint32_t esp;	//kernel stack address
	int sysnum;		//syscall number
	list<unsigned int> *p_retstack;	//retstack
	list<unsigned int> *p_callstack;	//callstack
};

extern uint32_t g_pc;
FILE *output_database;


void cur_dump_rets(FILE *file, unsigned int pc)
{
//#ifdef DEBUG
	fprintf(file, "%p rets:", p_cur_retstack);
	for(std::list<unsigned int>::iterator it = (*p_cur_retstack).begin(); 
			it != (*p_cur_retstack).end(); ++it){
		fprintf(file, "%x->", *it);
	}
	fprintf(file, "%x\n", pc);
//#endif
}

void cur_insert_retaddr(unsigned int pc)
{
	(*p_cur_retstack).push_back(pc);
}

int cur_delete_retaddr(unsigned int pc)
{
	(*p_cur_retstack).pop_back();
	if((*p_cur_retstack).size() == 0) {
		return 1;	//callstack empty;	
	}
	return 0;
}

void cur_dump_callstack(FILE *file, unsigned int pc)
{
	fprintf(file, "%p rets:", p_cur_retstack);
	for(std::list<unsigned int>::iterator it = (*p_cur_retstack).begin(); 
			it != (*p_cur_retstack).end(); ++it){
		fprintf(file, "%x->", *it);
	}
	fprintf(file, "%x\n", pc);
#if 0	
	fprintf(file, "%p rets:", p_cur_callstack);
	for(std::list<unsigned int>::iterator it = (*p_cur_callstack).begin(); 
			it != (*p_cur_callstack).end(); ++it){
		fprintf(file, "%x->", *it);
	}
	fprintf(file, "%x\n", pc);
#endif
}

void cur_insert_callstack(unsigned int pc)
{
	(*p_cur_callstack).push_back(pc);
}

int cur_delete_callstack(unsigned int pc)
{
	(*p_cur_callstack).pop_back();
	if((*p_cur_callstack).size() == 0) {
		return 1;	//callstack empty;	
	}
	return 0;
}


unsigned int get_parent_callstack(void)
{
	if((*p_cur_callstack).size() >= 2) {
		list<unsigned int>::reverse_iterator rit = (*p_cur_callstack).rbegin();
		return *(++rit);
	} else {
		return 0xffffffff;
	}
}

unsigned int get_cur_callstack(void)
{
	return (*p_cur_callstack).back();
}

void new_proc_start(uint32_t pc_start) 
{
	uint32_t new_esp = 0, old_esp = 0;
	if(disas_one_inst_ex(pc_start, &pemu_inst) == XED_ERROR_NONE) {
		xed_reg_enum_t reg_id_0;
		xed_reg_enum_t reg_id_1;
		
		const xed_operand_t *op_0 = xed_inst_operand(
				xed_decoded_inst_inst(&pemu_inst.PEMU_xedd_g), 0);
		const xed_operand_t *op_1 = xed_inst_operand(
				xed_decoded_inst_inst(&pemu_inst.PEMU_xedd_g), 1);
		
		xed_operand_enum_t op_name_0 = xed_operand_name(op_0);
		xed_operand_enum_t op_name_1 = xed_operand_name(op_1);
		
		if(operand_is_reg(op_name_0, &reg_id_0)) {
			old_esp = PEMU_get_reg(XED_REG_ESP) & 0xffffe000;
		}

		if(operand_is_reg(op_name_1, &reg_id_1)) {
			new_esp = PEMU_get_reg(reg_id_1);
		} else if(operand_is_mem(op_name_1, &new_esp, 1)) {
			PEMU_read_mem(new_esp, 4, &new_esp);
		} else {
			return;
		}
		new_esp &= 0xffffe000;
	}

#ifdef DEBUG
	fprintf(stderr, "old:%x int:%d sched:%x new:%x\n", 
			old_esp, s_int_level, s_in_sched, new_esp);
#endif
	
	//save old proc info
	if(g_all_info.count(old_esp) > 0) {
		g_all_info[old_esp].int_level = s_int_level;
		g_all_info[old_esp].in_sched = s_in_sched;
		g_all_info[old_esp].in_syscall = s_in_syscall;
		g_all_info[old_esp].sysnum = s_sysnum;
		g_all_info[old_esp].trace = s_trace;
		g_all_info[old_esp].esp = s_esp;
		g_all_info[old_esp].p_retstack = p_cur_retstack;
		g_all_info[old_esp].p_callstack = p_cur_callstack;
	} else {
	}

	//set info from new proc
	if(g_all_info.count(new_esp) > 0) {
		s_int_level = g_all_info[new_esp].int_level;
		s_in_sched = g_all_info[new_esp].in_sched;
		s_in_syscall = g_all_info[new_esp].in_syscall;
		s_trace = g_all_info[new_esp].trace;
		s_esp = g_all_info[new_esp].esp;
		s_sysnum = g_all_info[new_esp].sysnum;
		p_cur_retstack = g_all_info[new_esp].p_retstack;
		p_cur_callstack = g_all_info[new_esp].p_callstack;
#ifdef DEBUG
		fprintf(stderr, "new:%x int:%d sched:%x in_syscall:%x esp:%x\n",
				new_esp, s_int_level, s_in_sched, s_in_syscall, s_esp);
#endif
	} else {//if we are not at syscall context of new proc or we didn't trace it.
		s_int_level = 0;
		s_in_sched = 0;
		s_in_syscall = 0;
		s_trace = 0;
		s_esp = 0;
		s_sysnum = -1;
		p_cur_retstack = NULL;
		p_cur_callstack = NULL;
#ifdef DEBUG
		fprintf(stderr, "esp not found in new_proc_start: %x\n", new_esp);
#endif
	}
}

/*the interrupt ocurs at syscall context??*/
int syscall_intr(uint32_t esp)
{
	return g_all_info.count(esp);
}

/*the iret ocurs at syscall context??*/
int syscall_iret(uint32_t esp)
{
	return g_all_info.count(esp);
}

/*enter syscall*/
void syscall_enter(uint32_t esp, int label)
{
//	if(PEMU_get_reg(XED_REG_EAX) == 0xfc
//			|| PEMU_get_reg(XED_REG_EAX) == 0x1) //ignore sys_exit
//		return;
	
#ifdef DEBUG
	if(g_all_retstacks.count(esp) != 0) {
		p_cur_retstack = &g_all_retstack[esp];
		cur_dump_rets(stdout, 0);
		printf("fuch shit here esp:%x %x %x %x %x %x %p\n",
				g_all_info[esp].in_syscall,
				g_all_info[esp].int_level,
				g_all_info[esp].in_sched,
				g_all_info[esp].trace,
				g_all_info[esp].esp,
				g_all_info[esp].sysnum,
				g_all_info[esp].p_retstack);

		printf("syscall_enter: %d %x %d %x label:%d\n", 
				s_int_level, 
				s_in_sched,
				s_in_syscall,
				s_esp,
				label);
		assert(g_all_retstacks.count(esp) == 0);
	}

	printf("syscall_enter:%x label:%x sysnum:%x\n", 
			esp, label, PEMU_get_reg(XED_REG_EAX));
	
	assert(g_all_retstacks.count(esp) == 0);
	assert(g_all_info.count(esp) == 0);
#endif

	struct info info;
	info.int_level = 0;
	info.in_sched = 0;
	info.in_syscall = 1;
	info.trace = 0;
	info.esp = esp;
	info.sysnum = PEMU_get_reg(XED_REG_EAX);
	g_all_info[esp] = info;
	g_all_retstacks[esp] = list<unsigned int>(0, 0);
	g_all_callstacks[esp] = list<unsigned int>(0,0);

//	if(PEMU_get_reg(XED_REG_EAX) == 0xa2) {
//		info.trace = 1;
//	}

	s_int_level = 0;
	s_in_sched = 0;
	s_trace = info.trace;
	s_sysnum = info.sysnum;
	s_in_syscall = 1;
	s_esp = esp;
	p_cur_retstack = &g_all_retstacks[esp];
	p_cur_callstack = &g_all_callstacks[esp];
	(*p_cur_retstack).clear();
	(*p_cur_callstack).clear();
}

/*leave syscall*/
void syscall_exit(uint32_t esp, int label)
{
#ifdef DEBUG
	printf("syscall_exit: %x %d %x %d %x label:%d\n", 
			esp, 
			s_int_level, 
			s_in_sched,
			s_in_syscall,
			s_esp,
			label);
	
	if(p_cur_retstack != NULL) {
		if((*p_cur_retstack).size() != 0 
				&& (*p_cur_retstack).back() != 0xc1031232) {
			for(std::list<unsigned int>::iterator it = (*p_cur_retstack).begin(); 
					it != (*p_cur_retstack).end(); ++it) {
				fprintf(stderr, "%x->", *it);
			}
			assert((*p_cur_retstack).size() == 0);
		}
		goto OUT;
	}
	assert(s_int_level == 0);
	assert(s_in_sched == 0);
	//assert(s_esp == esp);
OUT:
#endif

	g_all_retstacks.erase(esp);
	g_all_callstacks.erase(esp);
	g_all_info.erase(esp);
	
	p_cur_retstack = NULL;
	p_cur_callstack = NULL;
	s_in_sched = 0;
	s_int_level = 0;
	s_trace = 0;
	s_in_syscall = 0;
	s_sysnum = -1;
	s_esp = 0;

}


void decr_intr(void)
{
	s_int_level--;
#ifdef DEBUG
	printf("decr esp:%x int:%d\n", get_kernel_esp() & 0xffffe000, s_int_level);
#endif
}

void inc_intr(void)
{
	s_int_level++;
#ifdef DEBUG
	printf("inc esp:%x int:%d\n", get_kernel_esp() & 0xffffe000, s_int_level);
#endif
}

int is_cur_retaddr(unsigned int pc)
{
	if((*p_cur_retstack).empty())
		return 0;
	if((*p_cur_retstack).back() == pc){
		//g_ret_stack.pop();
		return 1;
	}else {
		return 0;
	}
}

///////////////////output info////////////////////////////////////
struct DataIntem
{
	unsigned int pc;
	int size;
	char name[30];
};

map<int, unordered_set<size_t> > sys_read;
map<int, unordered_set<size_t> > sys_write;
map<int, unordered_set<size_t> > sys_create;
map<int, unordered_set<size_t> > sys_delete;
map<size_t, unordered_set<string> > delete_loc_off;
map<size_t, unordered_set<string> > read_loc_off;
map<size_t, unordered_set<string> > write_loc_off;
map<size_t, struct DataIntem> database;

map<size_t, map<int, unordered_set<string> > > read_off_type;
map<size_t, map<int, unordered_set<string> > > write_off_type;
map<size_t, map<int, string> > first_write_off_type;

map<size_t, int> vmalloc_database;

void save_vmalloc(size_t type)
{
	if(vmalloc_database.count(type) == 0) {
		cur_dump_callstack(output_database, g_pc);
		vmalloc_database[type] = 1;
	}
}

void set_readSys(int sysnum, unsigned int addr, NodeType *tmp)
{
	stringstream ss;
	sys_read[sysnum].insert(tmp->type);
	//ss << hex << hash_read_callstack(g_pc) << ":" << addr - tmp->key;
	//ss << hex << g_pc << ":" << addr - tmp->key;
	//read_loc_off[tmp->type].insert(ss.str());
	
	ss << hex << hash_read_callstack(g_pc) << ":" << g_pc;
	read_off_type[tmp->type][addr-tmp->key].insert(ss.str());
}

void set_writeSys(int sysnum, unsigned int addr, NodeType *tmp)
{
	stringstream ss;
	sys_write[sysnum].insert(tmp->type);
	//ss << hex << hash_write_callstack(g_pc) << ":" << addr - tmp->key;
	//ss << hex << g_pc << ":" << addr - tmp->key;
	//write_loc_off[tmp->type].insert(ss.str());

	ss << hex << hash_write_callstack(g_pc) << ":" << g_pc;
	if(first_write_off_type.count(tmp->type) == 0
			|| first_write_off_type[tmp->type].count(addr-tmp->key) == 0) {
		first_write_off_type[tmp->type][addr-tmp->key] = ss.str();
	}
	write_off_type[tmp->type][addr-tmp->key].insert(ss.str());

}

void update_traced_size(size_t obj, int size)
{
	if(database.count(obj) != 0 && database[obj].size < size) {
		database[obj].size = size;
		//cout<<"update "<<hex<<obj<<" "<<size<<endl;
	}
}

void set_createSys(int sysnum, size_t obj, struct DataIntem data)
{
	sys_create[sysnum].insert(obj);

	if(database.count(obj) == 0) {
		cur_dump_callstack(output_database, g_pc);
		fprintf(output_database, "%lx %x %s %d\n", 
				obj, data.pc, data.name, data.size);
		database[obj] = data;
	}
}

void set_deleteSys(int sysnum, NodeType *tmp)
{
	if(in_syscall_context()) {
		if(s_in_sched == 0 && s_int_level == 0) {
			stringstream ss;
			sys_delete[sysnum].insert(tmp->type);
			ss << hex << hash_delete_callstack(g_pc);
			delete_loc_off[tmp->type].insert(ss.str());
		}
	}
}


void set_dump_one(char *fname, map<int, unordered_set<size_t> >& sys)
{
	FILE *file = fopen(fname, "w");
	
	if(!file) {
		fprintf(stderr, "error in open %s\n", fname);
		exit(0);
	}
	for(map<int, unordered_set<size_t> >::iterator it = sys.begin();
			it != sys.end(); it++) {
			fprintf(file, "%d_%s:", 
					it->first, g_syscall[it->first].c_str());
		for(auto it2 = it->second.begin();
				it2 != it->second.end(); it2++) {
			fprintf(file, " %lx", *it2);
		}
		fprintf(file, "\n");
	}
	fclose(file);
}

void dump_loc_off(char *fname, map<size_t, unordered_set<string> > &loc_off)
{
	FILE *file = fopen(fname, "w");
	
	if(!file) {
		fprintf(stderr, "error in open %s\n", fname);
		exit(0);
	}
	for(map<size_t, unordered_set<string> >::iterator it = loc_off.begin();
			it != loc_off.end(); it++) {
			fprintf(file, "%lx", it->first);
		for(auto it2 = it->second.begin();
				it2 != it->second.end(); it2++) {
			fprintf(file, " %s", (*it2).c_str());
		}
		fprintf(file, "\n");
	}
	fclose(file);
}

void dump_hash_callstack(char *fname, map<size_t, string > &hash_callstack)
{
	FILE *file = fopen(fname, "w");
	
	if(!file) {
		fprintf(stderr, "error in open %s\n", fname);
		exit(0);
	}
	for(map<size_t, string >::iterator it = hash_callstack.begin();
			it != hash_callstack.end(); it++) {
		fprintf(file, "%lx:%s\n", it->first, it->second.c_str());
	}
	fclose(file);

}

void dump_type_off(char *fname, map<size_t, map<int, unordered_set<string> > > &hash)
{
	FILE *file = fopen(fname, "w");
	if(!file) {
		fprintf(stderr, "error in open %s\n", fname);
		exit(0);
	}
	for(map<size_t, map<int, unordered_set<string> > >::iterator it = hash.begin(); it != hash.end();
			it++) {
		fprintf(file, "%lx", (*it).first);
		for(map<int, unordered_set<string> >::iterator it2 = (*it).second.begin(); it2 != (*it).second.end();
				it2++) {
			for(auto it3 = (*it2).second.begin(); it3 != (*it2).second.end(); it3++) {
				fprintf(file, " %x:%s", (*it2).first, (*it3).c_str());
			}
		}
		fprintf(file, "\n");
	}
	fclose(file);
}

void dump_first_type_off(char *fname, map<size_t, map<int, string> > &hash)
{
	FILE *file = fopen(fname, "w");
	if(!file) {
		fprintf(stderr, "error in open %s\n", fname);
		exit(0);
	}
	for(map<size_t, map<int, string> >::iterator it = hash.begin(); it != hash.end();
			it++) {
		fprintf(file, "%lx", (*it).first);
		for(map<int, string>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end();
				it2++) {
			fprintf(file, " %x:%s", (*it2).first, (*it2).second.c_str());
		}
		fprintf(file, "\n");
	}
	fclose(file);
}

void dump_database(void)
{
	FILE *file = fopen("database_size", "w");
	for(map<size_t, struct DataIntem>::iterator it = database.begin();
			it != database.end(); it++) {
		fprintf(file, "%llx %s %d\n", it->first, it->second.name, it->second.size-1);
	}
	fclose(file);
}

void open_database(void)
{
	output_database = fopen("database", "w");
	if(!output_database) {
		fprintf(stderr, "error in open database\n");
		exit(0);
	}
}

#if 0
FILE *modules;
void load_modules(void)
{
	modules = fopen("modules", "r");
	if(modules != NULL) {
		/**/
		fclose(modules);
	}
}
#endif

#if 0
map<string,int> names;
void record_name(char *name)
{
	string t(name);
	names[t] = 1;
}

void dump_names(char *fname, map<string,int> &names)
{
	FILE *file = fopen(fname, "w");
	
	if(!file) {
		fprintf(stderr, "error in open %s\n", fname);
		exit(0);
	}
	for(map<string, int >::iterator it = names.begin();
			it != names.end(); it++) {
		fprintf(file, "%s\n", it->first.c_str());
	}
	fclose(file);
}
#endif

void set_dump(void)
{
	set_dump_one("sys_read", sys_read);
	set_dump_one("sys_write", sys_write);
	set_dump_one("sys_create", sys_create);
	set_dump_one("sys_delete", sys_delete);
	dump_type_off("read_off_type", read_off_type);
	dump_type_off("write_off_type", write_off_type);
	dump_first_type_off("first_write_off_type", first_write_off_type);
	//dump_loc_off("read_loc_off", read_loc_off);
	//dump_loc_off("write_loc_off", write_loc_off);
	dump_loc_off("delete_loc_off", delete_loc_off);
	dump_hash_callstack("read_hash_callstack", read_hash_callstack);
	dump_hash_callstack("write_hash_callstack", write_hash_callstack);
	dump_hash_callstack("delete_hash_callstack", delete_hash_callstack);
	dump_database();
	//dump_names("names.s", names);
}


void set_syscall_call_funcs(unsigned int pc)
{
	syscall_call_funcs[pc]++;
}

void print_syscall_call_funcs(void)
{
	for(map<unsigned int,int>::iterator it = syscall_call_funcs.begin()
			;it != syscall_call_funcs.end();it++) {
		fprintf(stdout, "%x %x\n", it->first, it->second);
	}
}

map<unsigned int, unordered_set<unsigned int> > call_relation;

void set_call_relation(unsigned int caller, unsigned int callee)
{
	//printf("%x:%x\n", get_parent_callstack(), callee);
	call_relation[caller].insert(callee);
}

void print_call_relation(void)
{
	FILE *file = fopen("call_relation", "w");
	if(!file) {
		fprintf(stderr, "error in open %s\n", "call_relation");
		exit(0);
	}
	for(map<unsigned int, unordered_set<unsigned int> >::iterator it = call_relation.begin(); it != call_relation.end(); it++) {
		for(auto it3 = (*it).second.begin(); it3 != (*it).second.end(); it3++) {
				fprintf(file, "%x:%x\n", (*it).first, (*it3));
		}
	}

}

}
