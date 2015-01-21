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

static inline void set_mem_taint2( Addr a, unsigned int bytes)
{
   SecMap* sm;
   UInt    sm_off;
   ENSURE_MAPPABLE_BYTE_GRANUITY(ii_primary_map, a);
   sm    = ii_primary_map[a >> 16];

   sm_off = a & 0xFFFF;


//#ifdef DEBUG_TAINT
//      fprintf(stdout, "set_mem_taint2:\t%x\tval:%x\tval:%x\n", 
//		   a, bytes, sm->byte[sm_off]);
//#endif
   sm->byte[sm_off] = bytes;
}

void set_mem_taint_bysize2( Addr a, unsigned int bytes, UInt size)
{
	UInt i;

	for(i=0; i<size;i++)
		set_mem_taint2(a+i, bytes);

}


void taintInit2()
{
	init_shadow_memory();
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
