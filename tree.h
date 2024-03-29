#ifndef __TREE_H__
#define __TREE_H__

typedef struct tree {
    struct tree_node_base* root;
} Tree;

typedef struct tree_node_base {
    struct tree_node_base* left;
    struct tree_node_base* right;
} TreeNodeBase;

extern Tree tree_init(void);
#define Tree() (Tree){NULL}

extern void tree_deinit(Tree* const tree);

#endif // __TREE_H__
