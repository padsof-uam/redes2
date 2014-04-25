#include "G-2301-11-P2-list_helpers.h"
#include <stdlib.h>
#include <string.h>

void* int_duplicator(const void* a)
{
	const int* original = (const int*) a;
	int* b = (int*) malloc(sizeof(int));
	*b = *original;
	return b;
}

void* str_duplicator(const void* a)
{
	return (void*)strdup((const char*)a);
}

int str_comparator(const void* a, const void* b)
{
	return strcmp((const char*)a, (const char*)b);
}

int int_comparator(const void* a, const void* b)
{
	const int* ia = (const int*) a;
	const int* ib = (const int*) b;

	return *ia - *ib;
}

void* char_duplicator(const void* a)
{
	const char* original = (const char*) a;
	char* b = (char*) malloc(sizeof(char));
	*b = *original;
	return b;
}

int char_comparator(const void* a, const void* b)
{
	const char* ca = (const char*) a;
	const char* cb = (const char*) b;

	return *ca - *cb;
}

int ptr_comparator(const void* a, const void* b)
{
	return (int)a - (int)b;
}
