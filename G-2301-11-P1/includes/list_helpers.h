#ifndef LIST_HELPERS_H
#define LIST_HELPERS_H

typedef void* (*duplicator)(const void*);
typedef void (*destructor)(void*);
typedef int (*comparator)(const void*, const void *);
typedef int (*iterator_action)(const void* key, void* value, void* pass_through);

void* int_duplicator(const void* a);
int int_comparator(const void* a, const void* b);
void* str_duplicator(const void* str);
int str_comparator(const void* a, const void* b);
void* char_duplicator(const void* str);
int char_comparator(const void* a, const void* b);
#endif
