#include "database.h"
#include "utils.h"

#include <assert.h>
#include <sqlite3.h>
#include <stdio.h>
#include <time.h>

sqlite3 *database_init(void)
{
    sqlite3 *self;
    char *path = "database.db";

    bool is_fresh_start = !utils_path_exist(path);

    assert(sqlite3_open("database.db", &self) == 0);

    // Create tables
    const char *sql = "CREATE TABLE food ("
                      "id INTEGER NOT NULL UNIQUE,"
                      "created INTEGER NOT NULL,"
                      "name TEXT NOT NULL,"
                      "energy REAL NOT NULL,"
                      "PRIMARY KEY(id)"
                      ");";

    char *errmsg;
    if (is_fresh_start && sqlite3_exec(self, sql, NULL, NULL, &errmsg) != 0)
    {
        printf("SQL: %s\n", errmsg);
        sqlite3_free(errmsg);
        assert(0);
    }

    return self;
}

void database_deinit(sqlite3 *self)
{
    sqlite3_close(self);
}

void database_add_food(sqlite3 *self, char *name, float energy)
{
    int64_t created = time(NULL);
    char sql[256];
    sprintf(sql, "INSERT INTO food (created, name, energy) VALUES(%ld, \"%s\", %f)", created, name, energy);

    char *errmsg;
    if (sqlite3_exec(self, sql, NULL, NULL, &errmsg) != 0)
    {
        printf("SQL: %s\n", errmsg);
        sqlite3_free(errmsg);
        assert(0);
    }
}
