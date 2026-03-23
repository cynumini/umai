#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <raylib.h>

#include "paddings.h"
#include "sakana.h"

typedef struct
{
    bool has_value;
    u32 value;
} U32Optional;

U32Optional u32_optional_create(u32 value);

typedef struct
{
    bool has_value;
    Color value;
} ColorOptional;

ColorOptional color_optional_create(Color value);

typedef struct
{
    bool has_value;
    Paddings value;
} PaddingsOptional;

PaddingsOptional paddings_optional_create(Paddings value);

#endif // OPTIONAL_H