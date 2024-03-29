#include "string.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

String string_init(const char* cstr)
{
    assert(cstr != NULL);
    const size_t length = strlen(cstr);
    return (String){(uint8_t*)cstr, length};
}

bool string_empty(const String* const string)
{
    assert(string != NULL);
    return string->text == NULL || string->length == 0;
}

void string_print(const String* const string)
{
    assert(string != NULL);
    fwrite(string->text, string->length, 0, stdout);
}
