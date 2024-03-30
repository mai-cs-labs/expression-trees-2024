#include "tree.h"

#include <assert.h>
#include <stdlib.h>

static void tree_clean(TreeNode* const node);

Tree tree_init(void)
{
    return (Tree){NULL};
}

void tree_deinit(Tree* const tree)
{
    assert(tree != NULL);
    tree_clean(tree->root);
    tree->root = NULL;
}

void* tree_node__create(const size_t size)
{
    assert(size > 0);

    TreeNode* node = malloc(size);
    if (node == NULL)
        return NULL;

    node->left = NULL;
    node->right = NULL;

    return (void*)node;
}

void tree_node_destroy(TreeNode* const node)
{
    assert(node != NULL);
    free(node);
}

bool tree_empty(const Tree* const tree)
{
    assert(tree != NULL);
    return tree->root == NULL;
}

static void tree_clean(TreeNode* const node)
{
    if (node == NULL)
        return;

    tree_clean(node->left);
    tree_clean(node->right);
    tree_node_destroy(node);
}

