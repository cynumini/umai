#ifndef LIST_H
#define LIST_H

#include "arena.h"

#define LIST_DEFINE(NAME, PREFIX, TYPE)                                                                                \
    typedef struct NAME                                                                                                \
    {                                                                                                                  \
        TYPE value;                                                                                                    \
        struct NAME *next;                                                                                             \
    } NAME;                                                                                                            \
                                                                                                                       \
    inline void PREFIX##_add(Arena *arena, NAME *list, TYPE value)                                                     \
    {                                                                                                                  \
        if (list == NULL)                                                                                              \
        {                                                                                                              \
            list = ARENA_PUSH_STRUCT_ZERO(arena, NAME);                                                                \
            list->value = value;                                                                                       \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            NAME *current = list;                                                                                      \
            while (current->next != NULL)                                                                              \
            {                                                                                                          \
                current = current->next;                                                                               \
            }                                                                                                          \
            current->next = ARENA_PUSH_STRUCT_ZERO(arena, NAME);                                                       \
            current->next->value = value;                                                                              \
        }                                                                                                              \
    }

#endif /* end of include guard: LIST_H */
