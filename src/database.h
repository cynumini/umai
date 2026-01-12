#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdint.h>

sqlite3 *database_init(void);
void database_deinit(sqlite3 *self);
void database_add_food(sqlite3 *self, char *name, float energy);

#endif /* end of include guard: DATABASE_H */
