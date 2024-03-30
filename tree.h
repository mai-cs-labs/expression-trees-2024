#ifndef __TREE_H__
#define __TREE_H__

#include <stddef.h>
#include <stdbool.h>

typedef struct tree {
    struct tree_node* root;
} Tree;

typedef struct tree_node {
    struct tree_node* left;
    struct tree_node* right;
} TreeNode;

#define tree_node(type) struct { \
    TreeNode base; \
    type data; \
}

extern Tree tree_init(void);
#define Tree() (Tree){NULL}

extern void tree_deinit(Tree* const tree);

extern void* tree_node__create(const size_t size);
#define tree_node_create(type) ((type*)tree_node__create(sizeof(type)))

extern void tree_node_destroy(TreeNode* const node);

extern bool tree_empty(const Tree* const tree);

#endif // __TREE_H__
