#ifndef VEC_H
#define VEC_H
#include <stdlib.h>

typedef struct vector {
    void* base;
    size_t element_size;
    size_t allocated;
    size_t length;
}* Vec;

Vec new_vec(size_t element_size);

Vec vec_copy(Vec v);

void vec_free(Vec v);

size_t vec_length(Vec v);

void vec_add(Vec v, void* item);

// do not retain this across vec_add calls!
// Never returns NULL, even for empty lists
void* vec_items(Vec v);

void* vec_at(Vec v, size_t index);

#endif
