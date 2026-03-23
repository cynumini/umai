#include "paddings.h"

Paddings paddings_all(u32 value)
{
    return (Paddings){.top = value, .left = value, .bottom = value, .right = value};
}
