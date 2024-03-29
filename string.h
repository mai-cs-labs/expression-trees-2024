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

#define String(cstr) (String){(uint8_t*)(cstr), sizeof(cstr) - 1}

extern String string_create(const size_t length);
extern void string_destroy(String* const string);

extern bool string_empty(const String* const string);

extern String string_trim(const String* const string,
                          const size_t begin,
                          const size_t end);

extern size_t string_index(const String* const string, const uint8_t c);

extern void string_print(const String* const string);

#endif // __MEMORY_H__
