#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct string {
    uint8_t* text;
    size_t length;
} String;

extern String string_init(const char* cstr);
#define String(...) string_init(__VA_ARGS__)

extern bool string_empty(const String* const string);

extern void string_print(const String* const string);

#endif // __MEMORY_H__
