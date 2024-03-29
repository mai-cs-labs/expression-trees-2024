#include "list.h"

#include <assert.h>
#include <stdlib.h>

List list_init(void)
{
    return (List){NULL, NULL};
}

void list_deinit(List* const list)
{
    assert(list != NULL);
    list_clear(list);
}

void list_clear(List* const list)
{
    assert(list != NULL);

    ListNodeBase* curr = list->head;
    ListNodeBase* prev = NULL;

    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }

    if (prev == NULL)
        free(curr);

    list->head = NULL;
    list->tail = NULL;
}

void* list__insert_front(List* const list, const size_t size)
{
    assert(list != NULL);
    assert(size > 0);

    ListNodeBase* node = malloc(sizeof(ListNodeBase) + size);
    if (node == NULL)
        return NULL;

    node->next = list->head;
    node->prev = NULL;

    if (list->tail == NULL)
        list->tail = node;

    if (list->head != NULL)
        list->head->prev = node;

    list->head = node;

    return (void*)((char*)node + sizeof(ListNodeBase));
}

void* list__insert_back(List* const list, const size_t size)
{
    assert(list != NULL);
    assert(size > 0);

    ListNodeBase* node = malloc(sizeof(ListNodeBase) + size);
    if (node == NULL)
        return NULL;

    node->next = NULL;
    node->prev = list->tail;

    if (list->head == NULL)
        list->head = node;

    if (list->tail != NULL)
        list->tail->next = node;

    list->tail = node;

    return (void*)((char*)node + sizeof(ListNodeBase));
}
