//#define DEBUG_TAINT

#include "taint.h"
#include <stdlib.h>
#include "qemu-common.h"
/*---------------------shadow memory----------------------*/
#define PAGE_BITS 16
#define PAGE_SIZE (1<<PAGE_BITS)
#define PAGE_NUM  (1<<16)

#define IS_DISTINGUISHED_SM(smap) \
   ((smap) == &distinguished_secondary_map)

#define ENSURE_MAPPABLE(map, addr)                              \
   do {                                                           \
      if (IS_DISTINGUISHED_SM(map[addr >> 16])) {       \
         map[addr >> 16] = alloc_secondary_map; \
      }                                                           \
   } while(0)

#define ENSURE_MAPPABLE_BYTE_GRANUITY(map,addr)         \
   do {                                                           \
      if (IS_DISTINGUISHED_SM(map[addr >> 16])) {    \
          map[addr >> 16] = alloc_secondary_map(); \
      }                                                           \
   } while(0)


typedef struct {
   unsigned int byte[PAGE_SIZE];
} SecMap;

static SecMap distinguished_secondary_map;

static SecMap * ii_primary_map[PAGE_NUM];

static unsigned int shadow_bytes;


static void init_shadow_memory(void)
{
    Int i,j;

    for (i = 0; i< PAGE_SIZE; i++)
       distinguished_secondary_map.byte[i] = UNTAINTED; //0xff

      for (i = 0; i < PAGE_NUM; i++) {
        ii_primary_map[i] = &distinguished_secondary_map;
      }
}

static void free_shadow_memory(void)
{
    Int i,j;

      for (i = 0; i < PAGE_NUM; i++) {
        if(ii_primary_map[i] != &distinguished_secondary_map)
        {
			free(ii_primary_map[i]);
        }
      }
}


static SecMap* alloc_secondary_map (void)
{
   SecMap* map;
   UInt  i;

   /* Mark all bytes as invalid access and invalid value. */
   map = (SecMap *)malloc(sizeof(SecMap));
   shadow_bytes+=sizeof(SecMap);
   for (i = 0; i < PAGE_SIZE; i++)
      map->byte[i] = UNTAINTED; /* Invalid Value */

   return map;
}

unsigned int  get_mem_taint2( Addr a )
{
   
   SecMap* sm;
   sm= ii_primary_map[a>>16];

   UInt    sm_off = a & 0xFFFF;

#ifdef DEBUG_TAINT
   fprintf(stdout, "get_mem_taint2:\t%x\tval:%x\n", a, sm->byte[sm_off]);
#endif

   return  sm->byte[sm_off];
}

void  set_mem_taint2( Addr a, unsigned int bytes)
{
   SecMap* sm;
   UInt    sm_off;
   ENSURE_MAPPABLE_BYTE_GRANUITY(ii_primary_map, a);
   sm    = ii_primary_map[a >> 16];

   sm_off = a & 0xFFFF;

   sm->byte[sm_off] = bytes;

#ifdef DEBUG_TAINT
      fprintf(stdout, "set_mem_taint2:\t%x\tval:%x\thost:%x\n", 
		   a, bytes, sm->byte + sm_off);
#endif
}

void  set_mem_taint_bysize2( Addr a, unsigned int bytes, UInt size)
{
	UInt i;

	for(i=0; i<size;i++)
		set_mem_taint2(a+i, bytes);

}
/******************************************************************
* Shadow for register
******************************************************************/
static unsigned int regTaint[XED_REG_LAST];
static unsigned int FDregTaint[XED_REG_LAST];
static void regUntainted(void)
{
	int i;

	for( i=0; i< XED_REG_LAST;i++)
		regTaint[i]=UNTAINTED;


	//regTaint[XED_REG_ESP]=TAINTED;
	//regTaint[XED_REG_EBP]=TAINTED;
}
static void regUntainted_fd(void)
{
	int i;

	for( i=0; i< XED_REG_LAST;i++)
		FDregTaint[i]=UNTAINTED;

}

unsigned int get_reg_taint2(xed_reg_enum_t reg)
{

#ifdef DEBUG_TAINT
	fprintf(stdout, "get_reg_taint2\t%s\tval:%lx\n", xed_reg_enum_t2str(reg), 
			regTaint[reg]);
#endif

	return regTaint[reg];
}
unsigned int get_reg_taint_fd2(xed_reg_enum_t reg)
{

	return FDregTaint[reg];
}

void set_reg_taint2(xed_reg_enum_t reg, unsigned int bytes)
{
	regTaint[reg] = bytes;

#ifdef DEBUG_TAINT
	fprintf(stdout, "taint2 to reg %s:%x\n", xed_reg_enum_t2str(reg), bytes);
#endif

#if 0  
   //eax
	if(XED_REG_EAX == reg) {
		regTaint[(UInt)XED_REG_AX]=bytes;
		regTaint[(UInt)XED_REG_AH]=bytes;
		regTaint[(UInt)XED_REG_AL]=bytes;
    }
    //ebx
    else if(XED_REG_EBX == reg) {
		regTaint[(UInt)XED_REG_BX]=bytes;
		regTaint[(UInt)XED_REG_BH]=bytes;
		regTaint[(UInt)XED_REG_BL]=bytes;
	}
    //ecx
    else if(XED_REG_ECX == reg) {
		regTaint[(UInt)XED_REG_CX]=bytes;
		regTaint[(UInt)XED_REG_CH]=bytes;
		regTaint[(UInt)XED_REG_CL]=bytes;
	}
    //edx
	else if(XED_REG_EDX == reg) {
		regTaint[(UInt)XED_REG_DX]=bytes;
		regTaint[(UInt)XED_REG_DH]=bytes;
		regTaint[(UInt)XED_REG_DL]=bytes;
    }
	//esi
	else if(XED_REG_ESI == reg) {
		regTaint[(UInt)XED_REG_SI]=bytes;
    }
	//edi
	else if(XED_REG_EDI == reg) {
		regTaint[(UInt)XED_REG_DI]=bytes;
    }
#endif
	switch(reg) {
		case XED_REG_EAX:
			regTaint[(UInt)XED_REG_AX]=bytes;
			regTaint[(UInt)XED_REG_AH]=bytes;
			regTaint[(UInt)XED_REG_AL]=bytes;
			break;
		case XED_REG_EBX:
			regTaint[(UInt)XED_REG_BX]=bytes;
			regTaint[(UInt)XED_REG_BH]=bytes;
			regTaint[(UInt)XED_REG_BL]=bytes;
			break;
		case XED_REG_ECX:
			regTaint[(UInt)XED_REG_CX]=bytes;
			regTaint[(UInt)XED_REG_CH]=bytes;
			regTaint[(UInt)XED_REG_CL]=bytes;
			break;
		case XED_REG_EDX:
			regTaint[(UInt)XED_REG_DX]=bytes;
			regTaint[(UInt)XED_REG_DH]=bytes;
			regTaint[(UInt)XED_REG_DL]=bytes;
			break;
		case XED_REG_ESI:
			regTaint[(UInt)XED_REG_SI]=bytes;
			break;
		case XED_REG_EDI:
			regTaint[(UInt)XED_REG_DI]=bytes;
			break;
		default:
			break;
	}

#ifdef DEBUG_TAINT
	fprintf(stdout, "set_reg_taint2:\t%s\tval:%x\n", xed_reg_enum_t2str(reg), 
			bytes);
#endif

}
void set_reg_taint_fd2(xed_reg_enum_t reg, unsigned int bytes)
{
	FDregTaint[reg]=bytes;

#ifdef DEBUG_VMMI
  if(is_ins_log())
	   qemu_log("Fd Reg: %u  taint %x", reg, bytes);
#endif
	
   //eax
	if(XED_REG_EAX == reg) {
		FDregTaint[(UInt)XED_REG_AX]=bytes;
		FDregTaint[(UInt)XED_REG_AH]=bytes;
		FDregTaint[(UInt)XED_REG_AL]=bytes;
    }
    //ebx
    else if(XED_REG_EBX == reg) {
		FDregTaint[(UInt)XED_REG_BX]=bytes;
		FDregTaint[(UInt)XED_REG_BH]=bytes;
		FDregTaint[(UInt)XED_REG_BL]=bytes;
	}
    //ecx
    else if(XED_REG_ECX == reg) {
		FDregTaint[(UInt)XED_REG_CX]=bytes;
		FDregTaint[(UInt)XED_REG_CH]=bytes;
		FDregTaint[(UInt)XED_REG_CL]=bytes;
	}
    //edx
	else if(XED_REG_EDX == reg) {
		FDregTaint[(UInt)XED_REG_DX]=bytes;
		FDregTaint[(UInt)XED_REG_DH]=bytes;
		FDregTaint[(UInt)XED_REG_DL]=bytes;
    }
}


void taintInit2()
{
	//objInit();
	init_shadow_memory();
	regUntainted();
	regUntainted_fd();
}

#if 0
void mem_taint_format()
{
	Int i,j;

	for (i =0; i< PAGE_NUM; i++)
		for(j=0;j< PAGE_SIZE;j++)
			if(ii_primary_map[i]->byte[j]>TAINTED)
				ii_primary_map[i]->byte[j]=UNTAINTED;

}
#endif
