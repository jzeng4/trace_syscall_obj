#ifndef DISAS_H
#define DISAS_H

#include <xed-interface.h>
#include "pemu.h"
#include "qemu-pemu.h"

#include "qemu-common.h"

xed_error_enum_t disas_one_inst_ex(target_ulong pc, struct PEMU_INST *inst);
#endif
