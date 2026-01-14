#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

bool utils_path_exist(const char *path);
char *read_text(const char *path);
void __unreachable(const char *file, int line, const char *func);

#define DYNAMIC_ARRAY_INIT(TYPE, NAME)                                                                                 \
    typedef struct NAME                                                                                                \
    {                                                                                                                  \
        TYPE *items;                                                                                                   \
        size_t capacity;                                                                                               \
        size_t len;                                                                                                    \
    } NAME

#define DYNAMIC_ARRAY_ADD(ARRAY, VALUE)                                                                                \
    {                                                                                                                  \
        size_t index = ARRAY->len;                                                                                     \
        if (index >= ARRAY->capacity)                                                                                  \
        {                                                                                                              \
            if (ARRAY->capacity == 0)                                                                                  \
                ARRAY->capacity = 8;                                                                                   \
            else                                                                                                       \
                ARRAY->capacity *= 2;                                                                                  \
            ARRAY->items = realloc(ARRAY->items, sizeof(VALUE) * ARRAY->capacity);                                     \
        }                                                                                                              \
        ARRAY->items[index] = VALUE;                                                                                   \
        ARRAY->len++;                                                                                                  \
    }

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define UNREACHABLE __unreachable(__FILE__, __LINE__, __func__)
#define UNREACHABLE_RETURN(r)                                                                                          \
    __unreachable(__FILE__, __LINE__, __func__);                                                                       \
    return r

#endif /* end of include guard: UTILS_H */
