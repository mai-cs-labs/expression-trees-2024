#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>

typedef struct list {
    struct list_node_base* head;
    struct list_node_base* tail;
} List;

typedef struct list_node_base {
    struct list_node_base* next;
    struct list_node_base* prev;
} ListNodeBase;

extern List list_init(void);
#define List(...) list_init(__VA_ARGS__)

extern void list_deinit(List* const list);

extern void list_clear(List* const list);

extern void* list__insert_front(List* const list, const size_t size);
#define list_insert_front(list_p, type) \
    ((type*)list__insert_front((list_p), sizeof(type)))

extern void* list__insert_back(List* const list, const size_t size);
#define list_insert_back(list_p, type) \
    ((type*)list__insert_back((list_p), sizeof(type)))

#define list_range(it, list) \
    (ListNodeBase* it = (list).head; (it) != NULL; (it) = (it)->next)

#define list_node_data(list_node_p, type) \
    ((type*)((char*)(list_node_p) + sizeof(ListNodeBase)))

#endif // __LIST_H__
