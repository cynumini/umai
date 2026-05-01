#ifndef SKN_DN
#define SKN_DN

#define DEFINE_DA(NAME, TYPE) \
typedef struct                \
{                             \
    TYPE *data;               \
    size_t len;               \
    size_t capacity;          \
} NAME                        \

#define da_append(ARENA, ARRAY, ITEM) do                                                      \
{                                                                                             \
    if ((ARRAY)->len >= (ARRAY)->capacity)                                                    \
    {                                                                                         \
        size_t old_capacity = (ARRAY)->capacity;                                              \
        if ((ARRAY)->capacity == 0) (ARRAY)->capacity = 1;                                    \
        (ARRAY)->capacity *= 2;                                                               \
        (ARRAY)->data = arena_realloc(ARENA, (ARRAY)->data, old_capacity, (ARRAY)->capacity); \
    }                                                                                         \
    (ARRAY)->data[(ARRAY)->len] = ITEM;                                                       \
    (ARRAY)->len++;                                                                           \
} while(0)

#endif // SKN_DN
