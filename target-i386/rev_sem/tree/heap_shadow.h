#ifndef __HEAP_SHADOW_H
#define __HEAP_SHADOW_H
///////////////////////////////////////////////////////////////////////

#if 0
typedef struct shadow_heap_node
{
#if 0
	unsigned int shadow_size;
	void * type;
	unsigned int start_addr;
#endif
	size_t type;
	int used_size;
	unsigned int ret;
} heap_shadow_node_t;
#endif

//////////////////////////////////////////////////////////////////////

//////////////////////
// supplied by user //
//////////////////////

typedef unsigned int KeyType;            // type of key


//typedef heap_shadow_node_t ValType;


// how to compare keys
//#define compLT(a,b) (a <= b)
//#define compL(a,b) (a < b)
#define compEQ(a,b) (a == b)

/////////////////////////////////////
// implementation independent code //
/////////////////////////////////////

typedef enum {
    RBT_STATUS_OK = 0,
    RBT_STATUS_MEM_EXHAUSTED,
    RBT_STATUS_DUPLICATE_KEY,
    RBT_STATUS_REALLOC_SMALL,
    RBT_STATUS_REALLOC_LARGE,
    RBT_STATUS_KEY_NOT_FOUND
} RbtStatus;

typedef enum { BLACK, RED } nodeColor;

typedef struct NodeTag {
	struct NodeTag *left;       // left child
    struct NodeTag *right;      // right child
    struct NodeTag *parent;     // parent
    nodeColor color;            // node color (BLACK, RED)

	KeyType key;                // key used for searching
    KeyType size;				// allocation size -->also used in searching
	size_t type;
	unsigned int pc;			// the address for the object's creation
	char obj_name[30];
} NodeType;

//RbtStatus rbtInsert(KeyType key, KeyType size, heap_shadow_node_t *p);
//NodeType *rbtFind2(KeyType key);
//void displayNode(FILE *file, NodeType *p);
#define UInt unsigned int
typedef void(*RB_CALLBACK)(NodeType*, void*);
//int ds_code_delete_rb(UInt start_addr);
//void ds_code_traverse(FILE *f);
void ds_code_traverse(RB_CALLBACK callback, void* p);
//int ds_code_insert_rb(UInt start_addr, UInt size, NodeType p);
void ds_code_load_rb(void);
NodeType *ds_code_rbtFind2(KeyType key);
#endif
