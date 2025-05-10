#include "vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SIZE_INCREMENT 20
// typedef struct vec {
// void* base;
// size_t element_size;
// size_t allocated;
// size_t length;
// } *Vec;
Vec new_vec(size_t element_size) {
    Vec v           = malloc(sizeof(struct vector));
    v->element_size = element_size;
    v->base         = NULL;
    return v;
}
Vec vec_copy(Vec v) {
    Vec nv            = malloc(sizeof(struct vector));
    nv->element_size  = v->element_size;
    nv->allocated     = v->allocated;
    nv->length        = v->length;
    size_t region_len = nv->element_size * nv->allocated;
    nv->base          = malloc(region_len);
    memcpy(nv->base, v->base, region_len);
    return nv;
}
void vec_free(Vec v) {
    if (v->base)
        free(v->base);
    free(v);
}
size_t vec_length(Vec v) {
    return v->length;
}
void vec_add(Vec v, void* item) {
    if (v->base == NULL) {
        v->allocated = SIZE_INCREMENT;
        v->base      = malloc(v->element_size * v->allocated);
        v->length    = 0;
    } else if (v->length == v->allocated) {
        v->allocated += SIZE_INCREMENT;
        v->base = realloc(v->base, v->element_size * v->allocated);
    }
    memcpy((char*)v->base + v->length * v->element_size, item, v->element_size);
    v->length++;
}
void* vec_items(Vec v) {
    // this keeps clients from having to worry about getting a NULL
    if (v->base == NULL) {
        v->allocated = SIZE_INCREMENT;
        v->base      = malloc(v->element_size * v->allocated);
        v->length    = 0;
    }
    return v->base;
}

void* vec_at(Vec v, size_t index) {
    if (v == NULL) {
        fprintf(stderr, "Vector is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (index >= v->length) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    }
    return (vec_items(v) + (index * v->element_size));
}
