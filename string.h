#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct string {
	uint8_t* text;
	size_t length;
	bool _allocated;
} String;

extern String string_init(const char* cstr);

#define String(cstr) (String){(uint8_t*)(cstr), sizeof(cstr) - 1, false}

extern String string_create(const size_t length);
extern void string_destroy(String* const string);

extern bool string_empty(const String* const string);

extern String string_trim(const String* const string,
                          const size_t begin,
                          const size_t end);

extern double string_to_double(const String* const string);

extern bool string_equal(const String* const lhs,
                         const String* const rhs);

extern void string_write(const String* const string, FILE* const file);
#define string_print(string_p) string_write((string_p), stdout)
#define string_debug_print(string_p) string_write((string_p), stderr)

#endif // __MEMORY_H__
