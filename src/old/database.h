#ifndef DATABASE_H
#define DATABASE_H

#include "utils.h"
#include <sqlite3.h>

typedef struct Food
{
    int64_t id;
    int64_t created;
    const char *name;
    float energy;
} Food;

DYNAMIC_ARRAY_INIT(Food, Foods);

sqlite3 *database_init(void);
void database_deinit(sqlite3 *self);
void database_add_food(sqlite3 *self, char *name, float energy);
Foods database_select_food(sqlite3 *self);
void database_deinit_food(Foods *foods);

#endif /* end of include guard: DATABASE_H */
