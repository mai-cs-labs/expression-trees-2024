#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>

typedef struct list {
	struct list_node* head;
	struct list_node* tail;
} List;

typedef struct list_node {
	struct list_node* next;
	struct list_node* prev;
} ListNode;

extern List list_init(void);
#define List() (List){NULL, NULL}

extern void list_deinit(List* const list);

extern void* list__insert_front(List* const list, const size_t size);
#define list_insert_front(list_p, type) \
	((type*)list__insert_front((list_p), sizeof(type)))

extern void* list__insert_back(List* const list, const size_t size);
#define list_insert_back(list_p, type) \
	((type*)list__insert_back((list_p), sizeof(type)))

extern void* list__insert_after(List* const list,
                                ListNode* const node,
                                const size_t size);
#define list_insert_after(list_p, node_p, type) \
	((type*)list__insert_after((list_p), (node_p), sizeof(type)))

#define list_range(it, list) \
	(ListNode* it = (list).head; (it) != NULL; (it) = (it)->next)

#define list_node_data(list_node_p, type) \
	((type*)((char*)(list_node_p) + sizeof(ListNode)))

#endif // __LIST_H__
