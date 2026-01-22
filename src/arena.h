#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct Arena
{
    void *data;
    void *current_position;
} Arena;

void *arena_push(Arena *arena, size_t size);
void *arena_push_zero(Arena *arena, size_t size);

#define ARENA_PUSH_STRUCT(ARENA, TYPE) (TYPE *)arena_push(ARENA, sizeof(TYPE))
#define ARENA_PUSH_STRUCT_ZERO(ARENA, TYPE) (TYPE *)arena_push_zero(ARENA, sizeof(TYPE))

#endif /* end of include guard: ARENA_H */
