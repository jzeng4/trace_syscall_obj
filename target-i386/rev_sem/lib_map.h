#ifndef CALL_STACK_H
#define CALL_STACK_H


void insert_callsite(unsigned int pc);
void delete_callsite(void);
void insert_retaddr(unsigned int pc);
int is_retaddr(unsigned int pc);
void insert_callstack(unsigned int pc);
void delete_callstack(void);
void dump_callsites(void);
void dump_callstacks(void);
void clear_calldata(void);
void insert_reference(unsigned int parent, unsigned child);
void taint_parent(unsigned int addr, unsigned int taint);
#endif
