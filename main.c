#include <stdio.h>

#include "string.h"
#include "list.h"

int main(void)
{
    List list = List();

    *list_insert_back(&list, String) = String("world");
    *list_insert_front(&list, String) = String(", ");
    *list_insert_back(&list, String) = String("!\n");
    *list_insert_front(&list, String) = String("Hello");

    for list_range(it, list) {
        String* string = list_node_data(it, String);
        string_print(string);
    }

    list_deinit(&list);
}
