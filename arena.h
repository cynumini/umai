#ifndef SKN_ARENA_H
#define SKN_ARENA_H

#include <stddef.h>
#include <stdint.h>

#define KB(SIZE) (SIZE << 10)
#define MB(SIZE) (KB(SIZE) << 10)

#ifdef DEBUG
#include <stdio.h>
#define LOG(FMT, ...) printf(FMT, __VA_ARGS__)
#else
#define LOG(FMT, ...)
#endif // DEBUG

typedef struct
{
    size_t position;
    size_t next_position;
    size_t size;
    uint8_t *data;
} Arena;

Arena
create_arena(size_t size);

void
delete_arena(Arena arena);

void *
arena_base_alloc(Arena *arena, size_t size, size_t align);

void *
arena_base_realloc(Arena *arena, void *ptr, size_t old_len, size_t new_len, size_t align);

#define arena_create(ARENA, TYPE) arena_base_alloc(ARENA, sizeof(TYPE), alignof(TYPE))
#define arena_alloc(ARENA, TYPE, LEN) arena_base_alloc(ARENA, LEN * sizeof(TYPE), alignof(TYPE))
#define arena_realloc(ARENA, PTR, OLD_LEN, NEW_LEN) arena_base_realloc(ARENA, PTR,                       \
                                                                       OLD_LEN * sizeof(typeof(PTR[0])), \
                                                                       NEW_LEN * sizeof(typeof(PTR[0])), \
                                                                       alignof(typeof(PTR[0])))

#ifdef SKN_ARENA_IMPLEMENTATION
#include <assert.h>
#include <stdlib.h>
#include <string.h>

Arena
create_arena(size_t size)
{
    return (Arena){ .size = size, .data = malloc(size) };
}

void
delete_arena(Arena arena)
{
    free(arena.data);
}

void *
arena_base_alloc(Arena *arena, size_t size, size_t align)
{
    size_t shift = 0;
    size_t mod = arena->next_position % align;
    if (mod != 0) shift = align - mod;
    arena->position =  arena->next_position + shift;
    assert(arena->position >= arena->next_position);
    assert((arena->position % align) == 0);
    arena->next_position = arena->position + size;
    assert(arena->next_position <= arena->size);
    memset(arena->data + arena->position, 0, size);
    LOG("arena_base_alloc: size = %li, from position = %li, to position = %li, used = %ld%%\n",
        size, arena->position, arena->next_position,
        ((arena->next_position * 100) / arena->size));
    return arena->data + arena->position;
}

void *
arena_base_realloc(Arena *arena, void *ptr, size_t old_len, size_t new_len, size_t align)
{
    void *new_ptr = ptr;
    if (ptr == (arena->data + arena->position))
    {
        arena->next_position = arena->position + new_len;
        assert(arena->next_position <= arena->size);
        LOG("arena_base_realloc: size = %li, from position = %li, to position = %li, used = %ld%%\n",
            new_len, arena->position, arena->next_position,
            ((arena->next_position * 100) / arena->size));
    }
    else
    {
        new_ptr = arena_base_alloc(arena, new_len, align);
        memcpy(new_ptr, ptr, old_len);
    }
    return new_ptr;
}
#endif // SKN_ARENA_IMPLEMENTATION
#endif // SKN_ARENA_H
