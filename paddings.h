#include "sakana.h"

typedef struct Paddings
{
    u32 top;
    u32 left;
    u32 bottom;
    u32 right;
} Paddings;

Paddings paddings_all(u32 value);
