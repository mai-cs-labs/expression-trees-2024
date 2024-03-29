#include "tree.h"

#include <assert.h>
#include <stdlib.h>

Tree tree_init(void)
{
    return (Tree){NULL};
}

static void tree_node_deinit(TreeNodeBase* const node)
{
    if (node == NULL)
        return;

    tree_node_deinit(node->left);
    tree_node_deinit(node->right);
    free(node);
}

void tree_deinit(Tree* const tree)
{
    assert(tree != NULL);
    tree_node_deinit(tree->root);
    tree->root = NULL;
}
