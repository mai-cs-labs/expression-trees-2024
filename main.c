#include <stdio.h>

#include "string.h"

int main(void)
{
    String message = String("Hello, world!\n");
    string_print(&message);
}
