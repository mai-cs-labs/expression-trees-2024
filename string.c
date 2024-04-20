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
	return (String){(uint8_t*)cstr, length, false};
}

String string_create(const size_t length)
{
	assert(length > 0);

	uint8_t* const text = malloc(length);
	if (text == NULL)
		return (String){NULL, 0, false};

	memset(text, 0, length);

	return (String){text, length, true};
}

void string_destroy(String* const string)
{
	assert(string != NULL);

	if (string->_allocated && string->text)
		free(string->text);

	string->text = NULL;
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

	return (String){string->text + begin, length, false};
}

double string_to_double(const String* const string)
{
	assert(string != NULL);

	char* const temp = alloca(string->length + 1);
	temp[string->length] = '\0';

	memcpy(temp, string->text, string->length);

	return atof(temp);
}

bool string_equal(const String* const lhs,
                  const String* const rhs)
{
	assert(lhs != NULL);
	assert(rhs != NULL);

	if (lhs->length != rhs->length)
		return false;

	return memcmp(lhs->text, rhs->text, lhs->length) == 0;
}

void string_write(const String* const string, FILE* const file)
{
	assert(string != NULL);
	fwrite(string->text, sizeof(uint8_t), string->length, file);
}
