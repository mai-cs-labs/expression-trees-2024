#include "string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <alloca.h>

String string_init(const char* cstr)
{
    assert(cstr != NULL);
    const size_t length = strlen(cstr);
    return (String){(uint8_t*)cstr, length};
}

String string_create(const size_t length)
{
    assert(length > 0);

    uint8_t* text = malloc(length);
    if (text == NULL)
        return (String){NULL, 0};

    memset(text, 0, length);

    return (String){text, length};
}

void string_destroy(String* const string)
{
    assert(string != NULL);

    if (string->text) {
        free(string->text);
        string->text = NULL;
    }

    string->length = 0;
}

bool string_empty(const String* const string)
{
    assert(string != NULL);
    return string->text == NULL || string->length == 0;
}

String string_trim(const String* const string,
                   const size_t begin,
                   const size_t end)
{
    assert(string != NULL);
    assert(begin <= string->length);
    assert(end <= string->length);

    const size_t length = end - begin;

    return (String){string->text + begin, length};
}

double string_to_double(const String* const string)
{
    assert(string != NULL);

    char* const temp = alloca(string->length + 1);
    temp[string->length] = '\0';

    memcpy(temp, string->text, string->length);

    return atof(temp);
}

void string_write(const String* const string, FILE* const file)
{
    assert(string != NULL);
    fwrite(string->text, string->length, 1, file);
}
