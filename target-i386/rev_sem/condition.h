#ifndef CONDITION_H
#define CONDITION_H

#include "cpu.h"


static inline int is_cf_set(uint32_t eflags){
	return eflags & CC_C ? 1 : 0;
}


static inline int is_pf_set(uint32_t eflags){
	return eflags & CC_P ? 1 : 0;
}


static inline int is_af_set(uint32_t eflags){
	return eflags & CC_A ? 1 : 0;
}


static inline int is_zf_set(uint32_t eflags){
	return eflags & CC_Z ? 1 : 0;
}


static inline int is_sf_set(uint32_t eflags){
	return eflags & CC_S ? 1 : 0;
}


static inline int is_of_set(uint32_t eflags){
	return eflags & CC_O ? 1 : 0;
}
#endif
