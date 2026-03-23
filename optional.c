#include "optional.h"

U32Optional u32_optional_create(u32 value) { return (U32Optional){.has_value = true, .value = value}; }

ColorOptional color_optional_create(Color value) { return (ColorOptional){.has_value = true, .value = value}; }

PaddingsOptional paddings_optional_create(Paddings value)
{
    return (PaddingsOptional){.has_value = true, .value = value};
}
