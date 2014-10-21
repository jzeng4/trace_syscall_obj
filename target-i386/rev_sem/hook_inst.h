
#ifndef HOOK_INST_H
#define HOOK_INST_H

#include <xed-interface.h>

#include <stdio.h>
#include <sys/user.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
//#include <xed-interface.h>
//#include "hook_inst.h"
//#include "qemu-common.h"
#include "cpu.h"
#include "pemu.h"

typedef const xed_inst_t * INS;
typedef void (*InstrumentFunction)(INS ins);

extern xed_reg_enum_t s_base_regid;

#endif
