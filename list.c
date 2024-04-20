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

	ListNode* curr = list->head;
	ListNode* prev = NULL;

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

	ListNode* node = malloc(sizeof(ListNode) + size);
	if (node == NULL)
		return NULL;

	node->next = list->head;
	node->prev = NULL;

	if (list->tail == NULL)
		list->tail = node;

	if (list->head != NULL)
		list->head->prev = node;

	list->head = node;

	return (void*)((char*)node + sizeof(ListNode));
}

void* list__insert_back(List* const list, const size_t size)
{
	assert(list != NULL);
	assert(size > 0);

	ListNode* node = malloc(sizeof(ListNode) + size);
	if (node == NULL)
		return NULL;

	node->next = NULL;
	node->prev = list->tail;

	if (list->head == NULL)
		list->head = node;

	if (list->tail != NULL)
		list->tail->next = node;

	list->tail = node;

	return (void*)((char*)node + sizeof(ListNode));
}

void* list__insert_after(List* const list,
                         ListNode* const node,
                         const size_t size)
{
	assert(list != NULL);
	assert(node != NULL);
	assert(size > 0);

	ListNode* next = malloc(sizeof(ListNode) + size);
	if (next == NULL)
		return NULL;

	next->prev = node;
	next->next = node->next;

	if (node == list->tail)
		list->tail = next;

	if (node->next != NULL)
		node->next->prev = next;

	node->next = next;

	return (void*)((char*)next + sizeof(ListNode));
}
