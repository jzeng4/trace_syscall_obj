#ifndef CONFIG_PEMU_H
#define CONFIG_PEMU_H

#define PEMU_DEBUG
#define TIMMER

#define LINUX_2_6_32_8_NO_TRACE
//#define LINUX_3_2_58_NO_TRACE
//#define LINUX_3_0_58



////////////////////////////////////////////////////////////////////////////////////

#ifdef LINUX_2_6_32_8_NO_TRACE
#define KMEM_CACHE_CREATE 0xc10ad689 /*address of kmem_cache_create*/
#define KMEM_CACHE_ALLOC 0xc10acb31 /*address of kmem_cache_alloc*/
#define KMEM_CACHE_FREE	0xc10ac7b2  /*address of kmem_cache_free*/
#define KFREE	0xc10ac856	/*kfree*/
//#define SCHED_ADDR 0xc12683ba  /*address of schedule function*/
//#define ESP_SWITCH_PONIT	0xc1268a60	/*esp switching point*/

#define KMEM_CACHE_ALLOC_TRACE 0x0        /*start address for kmem_cache_alloc_trace*/
#define __KMALLOC 0xc10ad425 /*start address for __kmalloc*/
#define __KMALLOC_TRACK_CALLER 0xc10ad341 /*start address for __kmalloc_track_caller*/

#define __VMALLOC_NODE_FLAGS	0xc10a25de	/*__vmalloc_node*/
#define VFREE					0xc10a1e5e	/*vfree*/

#define STACK_SIZE	0x2000	/*kernel stack size*/
#define KERNEL_ADDR_MIN		0xc0000000	/*min address of kernel space*/
#define KERNEL_ESP	0xc1c07f80-0x217c 	/*the address of esp field of TSS*/
//#define ____CACHE_ALLOC_START 0x0 /*start address for ____cache_alloc*/
//#define ____CACHE_ALLOC_END 0x0 /*start address for ____cache_alloc*/
//#define TRACE_KMALLOC 0xc10aa3d8 /*start address for trace_kmalloc*/
//#define __KMALLOC 0xc10ad425 /*start address for __kmalloc*/
//#define __KMALLOC_TRACK_CALLER 0xc10ad341 /*start address for __kmalloc_track_caller*/
#define FUNCTION_INTERFACE "objs-2.6.32.8/signature_2.6.32.s"
#define TYPE_64 "objs-2.6.32.8/type64_2.6.32.s"

#define TIME_INTERRUPT ret_addr == 0xc1003bf5
#define COMMON_INTERRUPT ret_addr == 0xc10038b0
#define COMMON_EXCEPTION ret_addr == 0xc1269ecb
#define HARD_CODE_SCHED1 pc >= 0xc1268a66 && pc <= 0xc1268a88
#define HARD_CODE_SCHED2 ret_addr >= 0xc1268a66 && ret_addr <= 0xc1268a88
#endif

#ifdef LINUX_3_0_58
#define KMEM_CACHE_CREATE 0xc10bf8c8 /*address of kmem_cache_create*/
#define KMEM_CACHE_ALLOC 0xc10bec0c /*address of kmem_cache_alloc*/
#define KMEM_CACHE_FREE	0xc10be6ca  /*address of kmem_cache_free*/
#define KFREE	0xc10bf313	/*kfree*/

#define KMEM_CACHE_ALLOC_TRACE 0xc10bf0f8        /*start address for kmem_cache_alloc_trace*/
#define __KMALLOC 0xc10bf177 /*start address for __kmalloc*/
#define __KMALLOC_TRACK_CALLER 0xc10bfc4e /*start address for __kmalloc_track_caller*/

#define __VMALLOC_NODE_FLAGS	0xc10b4e80	/*__vmalloc_node_flags*/
#define VFREE					0xc10b4c6b	/*vfree*/

#define STACK_SIZE	0x2000	/*kernel stack size*/
#define KERNEL_ADDR_MIN		0xc0000000	/*min address of kernel space*/
#define KERNEL_ESP	0xdfbf2840-0x217c 	/*the address of esp field of TSS*/

#define FUNCTION_INTERFACE "objs-3.0.52/signature_3.0.52.s"
#define TYPE_64 "objs-3.0.52/type64_3.0.52.s"

#define TIME_INTERRUPT ret_addr == 0xc12c0199
#define COMMON_INTERRUPT ret_addr == 0xc12c4670
#define COMMON_EXCEPTION ret_addr == 0xc12c03ff
#define HARD_CODE_SCHED1 pc >= 0xc12be3e7 && pc <= 0xc12be409
#define HARD_CODE_SCHED2 ret_addr >= 0xc12be3e7 && ret_addr <= 0xc12be409

#endif

#ifdef LINUX_3_2_58_NO_TRACE
#define KMEM_CACHE_CREATE 0xc10c8496 /*address of kmem_cache_create*/

#define KMEM_CACHE_ALLOC 0xc10c78e5 /*address of kmem_cache_alloc*/
#define KMEM_CACHE_FREE	0xc10c73bf  /*address of kmem_cache_free*/
#define KFREE	0xc10c7e52	/*kfree*/
#define KMEM_CACHE_ALLOC_TRACE 0x0        /*start address for kmem_cache_alloc_trace*/
#define __KMALLOC 0xc10c7fde /*start address for __kmalloc*/
#define __KMALLOC_TRACK_CALLER 0xc10c87f6 /*start address for __kmalloc_track_caller*/

#define __VMALLOC_NODE_FLAGS	0xc10bd459	/*__vmalloc_node_flags*/
#define VFREE					0xc10bd22b	/*vfree*/

#define STACK_SIZE	0x2000	/*kernel stack size*/
#define KERNEL_ADDR_MIN		0xc0000000	/*min address of kernel space*/
#define KERNEL_ESP	0xdfbf2800-0x217c 	/*the address of esp field of TSS*/

#define FUNCTION_INTERFACE "objs-3.2.58/signature_3.2.58.s"
#define TYPE_64 "objs-3.2.58/type64_3.2.58.s"

#define TIME_INTERRUPT ret_addr == 0xc12d7c59
#define COMMON_INTERRUPT ret_addr == 0xc12dc170
#define COMMON_EXCEPTION ret_addr == 0xc12d7e87
#define HARD_CODE_SCHED1 pc >= 0xc12d5ed6 && pc <= 0xc12d5ef8
#define HARD_CODE_SCHED2 ret_addr >= 0xc12d5ed6 && ret_addr <= 0xc12d5ef8

#endif


#ifdef FREEBSD_9_1
#define KMEM_CACHE_CREATE 0xc0d22a60 /*address of kmem_cache_create*/
#define KMEM_CACHE_ALLOC 0xc0d24630 /*address of kmem_cache_alloc*/
#define KMEM_CACHE_FREE	0xc0d24270  /*address of kmem_cache_free*/
#define SCHED_ADDR 0xc0ac8b50   /*?address of schedule function*/
#define STACK_SIZE	0x2000 	/*kernel stack size*/
#define ESP_SWITCH_PONIT	0xc0e23458	/*esp switching point*/
#define KERNEL_ADDR_MIN		0xc0000000	/*min address of kernel space*/
#define KFREE	0x0	/*kfree*/
#define KERNEL_ESP	0x0 	/*the address of esp field of TSS*/
#define ____CACHE_ALLOC_START 0x0 /*start address for ____cache_alloc*/
#define ____CACHE_ALLOC_END 0x0 /*start address for ____cache_alloc*/
#define TRACE_KMALLOC 0x0 /*start address for trace_kmalloc*/
#define __KMALLOC 0x0 /*start address for __kmalloc*/
#define __KMALLOC_TRACK_CALLER 0x0 /*start address for __kmalloc_track_caller*/
#define KMEM_CACHE_ALLOC_TRACE 0x0
#endif


//#define TRACE_SYSCALL_OBJ
//#define NOSCHEDULE
//#define MYCPUID
//#define INFER_SIZE

#define CALLSTACK

//for keep track of objs
#define TRACE_OBJ

//for access
#define RECORD_MEM_ACCESS
	#define RECORD_INSTANCE_ACCESS
//	#define RECORD_TYPE_ACCESS

//for rewards
//#define SEM_TYPES

//for traverse function
#define RECORD_POINT_TO
#define RECORD_TRAVERSE
#define RECORD_CALL_FUNC
//#define RECORD_GLOBAL
#endif
