#ifndef __UTILITY_H
#define __UTILITY_H

#define VGM_BYTE_INVALID   0xFF
#define TAINTED 1
#define UNTAINTED 0
#define FDTAINTED 2

#include <xed-interface.h>
/* Always 8 bits. */
typedef  unsigned char   UChar;
typedef    signed char   Char;
typedef           char   HChar; /* signfulness depends on host */
                                /* Only to be used for printf etc */

/* Always 16 bits. */
typedef  unsigned short  UShort;
typedef    signed short  Short;

/* Always 32 bits. */
#define UInt unsigned int
#define UINT unsigned int

typedef signed int Int;
typedef unsigned int Addr;


#ifdef CPLUSPLUS
extern "C" {
#endif
void taintInit();
unsigned int get_mem_taint( Addr a );
void set_mem_taint( Addr a, unsigned int bytes);
void set_reg_taint(xed_reg_enum_t reg, unsigned int bytes);
unsigned int get_reg_taint(xed_reg_enum_t reg);
void set_mem_taint_bysize( Addr a, unsigned int bytes, UInt size);

void taintInit2();
unsigned int get_mem_taint2( Addr a );
//void set_mem_taint2( Addr a, unsigned int bytes);
void set_reg_taint2(xed_reg_enum_t reg, unsigned int bytes);
unsigned int get_reg_taint2(xed_reg_enum_t reg);
void set_mem_taint_bysize2( Addr a, unsigned int bytes, UInt size);

#ifdef CPLUSPLUS
}
#endif


#endif
