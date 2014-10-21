//#include "rb_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "heap_shadow.h"
//#include "config_pemu.h"
//#include "taint.h"
#define SENTINEL &sentinel      // all leafs are sentinels
#include <assert.h>

#define UInt unsigned int

static NodeType sentinel = { SENTINEL, SENTINEL, 0, BLACK, 0};

static NodeType *root = SENTINEL; // root of red-black tree



#if 0
// find key
static NodeType *rbtFind(KeyType key) {
    NodeType *current;
    current = root;
    while(current != SENTINEL) {
        if(compEQ(key, current->key)) {
            return current;
        } else {
            current = compLT (key, current->key) ?
                current->left : current->right;
        }
    }
    return NULL;
}

// in-order walk of tree
static void rbtInorder(FILE *f, NodeType *p, void (callback)(NodeType *)) {
    if (p == SENTINEL) return;
    rbtInorder(f, p->left, callback);
    //callback(p);
    fprintf(f, "%x\t%x\t%x\n", p->val.start_addr, p->val.shadow_size, p->val.type);
	rbtInorder(f, p->right, callback);
}
#endif


extern unsigned int g_pc;
static RbtStatus rbtErase(NodeType * z);
static void rotateLeft(NodeType *x) {

    // rotate node x to left

    NodeType *y = x->right;

    // establish x->right link
    x->right = y->left;
    if (y->left != SENTINEL) y->left->parent = x;

    // establish y->parent link
    if (y != SENTINEL) y->parent = x->parent;
    if (x->parent) {
        if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
    } else {
        root = y;
    }

    // link x and y
    y->left = x;
    if (x != SENTINEL) x->parent = y;
}

static void rotateRight(NodeType *x) {

    // rotate node x to right

    NodeType *y = x->left;

    // establish x->left link
    x->left = y->right;
    if (y->right != SENTINEL) y->right->parent = x;

    // establish y->parent link
    if (y != SENTINEL) y->parent = x->parent;
    if (x->parent) {
        if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
    } else {
        root = y;
    }

    // link x and y
    y->right = x;
    if (x != SENTINEL) x->parent = y;
}

static void insertFixup(NodeType *x) {

    // maintain red-black tree balance
    // after inserting node x

    // check red-black properties
    while (x != root && x->parent->color == RED) {
        // we have a violation
        if (x->parent == x->parent->parent->left) {
            NodeType *y = x->parent->parent->right;
            if (y->color == RED) {

                // uncle is RED
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {

                // uncle is BLACK
                if (x == x->parent->right) {
                    // make x a left child
                    x = x->parent;
                    rotateLeft(x);
                }

                // recolor and rotate
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateRight(x->parent->parent);
            }
        } else {

            // mirror image of above code
            NodeType *y = x->parent->parent->left;
            if (y->color == RED) {

                // uncle is RED
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {

                // uncle is BLACK
                if (x == x->parent->left) {
                    x = x->parent;
                    rotateRight(x);
                }
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateLeft(x->parent->parent);
            }
        }
    }
    root->color = BLACK;
}

// insert new node (no duplicates allowed)
static RbtStatus rbtInsert(KeyType key, KeyType size, NodeType p) {
    NodeType *current, *parent, *x;
    // allocate node for data and insert in tree

    // find future parent
    current = root;
    parent = 0;
    while (current != SENTINEL) {
        if (key == current->key)
        {
			if(size == current->size)
				return RBT_STATUS_DUPLICATE_KEY;
			else if (size <= current->size)
				return RBT_STATUS_REALLOC_SMALL;
			else
				return RBT_STATUS_REALLOC_LARGE;
		}
        parent = current;
        current = key < current->key ?
            current->left : current->right;
    }

    // setup new node
    if ((x = (NodeType*)malloc (sizeof(*x))) == 0)
		return RBT_STATUS_MEM_EXHAUSTED;

    x->parent = parent;
    x->left = SENTINEL;
    x->right = SENTINEL;
    x->color = RED;

    x->key = key;
    x->size = size;
	x->type = p.type;
	//x->used_size = p.used_size;
	x->pc = p.pc;
	//x->esp = p.esp;
	strcpy(x->obj_name, p.obj_name);

	// insert node in tree
    if(parent) {
        if(key <= parent->key)
            parent->left = x;
        else
            parent->right = x;
    } else {
        root = x;
    }
    
	insertFixup(x);

    return RBT_STATUS_OK;
}

static RbtStatus rbtInsert2(KeyType key, KeyType size, NodeType p) {
	NodeType *dup;
	while(1) {
		dup = ds_code_rbtFind2(key);
		if(dup == NULL) {
			break;
		}
		//printf("dup:: %x %x %x\n", key, dup->key, dup->size);
		rbtErase(dup);
	}

	return rbtInsert(key, size, p);
}

static void deleteFixup(NodeType *x) {

    // maintain red-black tree balance
    // after deleting node x

    while (x != root && x->color == BLACK) {
        if (x == x->parent->left) {
            NodeType *w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateLeft (x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rotateRight (w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rotateLeft (x->parent);
                x = root;
            }
        } else {
            NodeType *w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateRight (x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rotateLeft (w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rotateRight (x->parent);
                x = root;
            }
        }
    }
    x->color = BLACK;
}

// delete node
static RbtStatus rbtErase(NodeType * z) {
    NodeType *x, *y;

    if (z->left == SENTINEL || z->right == SENTINEL) {
        // y has a SENTINEL node as a child
        y = z;
    } else {
        // find tree successor with a SENTINEL node as a child
        y = z->right;
        while (y->left != SENTINEL) y = y->left;
    }

    // x is y's only child
    if (y->left != SENTINEL)
        x = y->left;
    else
        x = y->right;

    // remove y from the parent chain
    x->parent = y->parent;
    if (y->parent)
        if (y == y->parent->left)
            y->parent->left = x;
        else
            y->parent->right = x;
    else
        root = x;

    if (y != z) {
        z->key = y->key;
        z->size = y->size;
		z->type = y->type;
		//z->used_size = y->used_size;
		z->pc = y->pc;
    }


    if (y->color == BLACK)
        deleteFixup (x);
    free (y);

    return RBT_STATUS_OK;
}


static void rbtInorder(NodeType *node, RB_CALLBACK callback, void* p) {
    if (node == SENTINEL) return;
    rbtInorder(node->left, callback, p);
    callback(node, p);
    //fprintf((FILE*)p, "%x\t%x\t%p\n", node->key, node->size, node->val);
	rbtInorder(node->right, callback, p);
}

// delete nodes depth-first
static void rbtDelete(NodeType *p) {
    if (p == SENTINEL) return;
    rbtDelete(p->left);
    rbtDelete(p->right);
    free(p);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////


struct DataIntem
{
	unsigned int pc;
	int size;
	char name[30];
};

// find key, also check the size
NodeType *ds_code_rbtFind2(KeyType key)
{
    NodeType *current;
    current = root;
    while(current != SENTINEL) {
        if(current->key <= key && key < current->key+current->size) {
            return current;
        } else {
            current = key <= current->key ?
                current->left : current->right;
        }
    }
    return NULL;
}

void ds_code_traverse(RB_CALLBACK callback, void* p)
{
	rbtInorder(root, callback, p);
}



extern int s_sysnum;
extern void ds_code_erase_all(KeyType key, KeyType size);
void ds_code_all_delete_rb(UInt start_addr)
{
	NodeType *p;

	if((p = ds_code_rbtFind2(start_addr)) != NULL) {
		set_deleteSys(s_sysnum, p);
		rbtErase(p);
		ds_code_erase_all(start_addr, p->size);
		return 1;
	} else {
		return 0;
	}

}

//type: related to callstack;
void ds_code_all_insert_rb(UInt start_addr, UInt size, NodeType p)
{
	NodeType * pt;
	RbtStatus code;
	struct DataIntem data;
	
	data.pc = p.pc;
	data.size = size;
	strcpy(data.name, p.obj_name);
	set_createSys(s_sysnum, p.type, data);

	//dump_rets(stdout, g_pc);
	if((code = rbtInsert2(start_addr, size, p)) != RBT_STATUS_OK) {
		fprintf(stderr, "error code: %d start_addr=%x size=%x\n", code, start_addr, size);
		assert(0);
	}
}



void ds_code_load_rb(void)
{
#if 0
	FILE *file = fopen("mem_range.log", "r");
	if(file == NULL){
		fprintf(stderr, "error in open mem_range\n");
		exit(0);
	}
	
	unsigned int addr, range;
	NodeType hp;
	while(fscanf(file, "%x\t%x\t%llx\t%x\t%x\t%x\t%x\t%s\n", 
			&hp.key, &hp.size, &hp.type, &hp.used_size,
			&hp.ret, &hp.cr3, &hp.esp, hp.obj_name) != EOF) {
		ds_code_all_insert_rb(hp.key, hp.size, hp);
	}

	fclose(file);
#endif
}

void ds_code_erase_all(KeyType key, KeyType size) {
	NodeType *dup;
	while(1) {
		dup = ds_code_rbtFind2(key);
		if(dup == NULL) {
			break;
		}
		//printf("ds_code_erase_all:: %x %x %x\n", key, dup->key, dup->size);
		rbtErase(dup);
	}
}

