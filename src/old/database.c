#include "database.h"
#include "utils.h"

#include <assert.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
                      ") STRICT;";

    char *errmsg;
    if (is_fresh_start && sqlite3_exec(self, sql, NULL, NULL, &errmsg) != SQLITE_OK)
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
    if (sqlite3_exec(self, sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        printf("SQL: %s\n", errmsg);
        sqlite3_free(errmsg);
        assert(0);
    }
}

// static int callback(void *data, int argc, char **argv, char **azColName)
// {
//     printf("Some data I passed: %s\n", (char *)data);
//     for (int i = 0; i < argc; i++)
//     {
//         printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//     }
//     printf("\n");
//     // data is just thing I pass
//     return 0;
// }

// void database_select_food(sqlite3 *self)
// {
//     const char *sql = "SELECT * FROM food";
//     char *data = "Some data";
//     char *errmsg;
//
//     if (sqlite3_exec(self, sql, callback, (void *)data, &errmsg) != SQLITE_OK)
//     {
//         printf("SQL: %s\n", errmsg);
//         sqlite3_free(errmsg);
//         assert(0);
//     }
// }

Foods database_select_food(sqlite3 *self)
{
    Foods foods = {0};

    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT * FROM food";
    int rc = sqlite3_prepare_v2(self, sql, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);
    rc = sqlite3_step(stmt);
    while (rc != SQLITE_DONE)
    {
        Food food;
        int column_count = sqlite3_column_count(stmt);
        for (int i = 0; i < column_count; i++)
        {
            int type = sqlite3_column_type(stmt, i);
            const char *name = sqlite3_column_name(stmt, i);
            switch (type)
            {
            case SQLITE_INTEGER: {
                int64_t value = sqlite3_column_int64(stmt, i);
                if (strcmp(name, "id") == 0)
                {
                    food.id = value;
                }
                else if (strcmp(name, "created") == 0)
                {
                    food.created = value;
                }
                else
                {
                    printf("%s(INTEGER) = %li\n", name, value);
                    UNREACHABLE;
                }
                break;
            }
            case SQLITE_TEXT: {
                const unsigned char *value = sqlite3_column_text(stmt, i);
                if (strcmp(name, "name") == 0)
                {
                    food.name = strdup((const char *)value);
                }
                else
                {
                    printf("%s(TEXT) = %s\n", name, value);
                    UNREACHABLE;
                }
                break;
            }
            case SQLITE_FLOAT: {
                double value = sqlite3_column_double(stmt, i);
                if (strcmp(name, "energy") == 0)
                {
                    food.energy = (float)value;
                }
                else
                {
                    printf("%s(FLOAT) = %f\n", name, value);
                    UNREACHABLE;
                }
                break;
            }
            default: {
                assert(0);
                break;
            }
            }
        }
        DYNAMIC_ARRAY_ADD((&foods), food)
        rc = sqlite3_step(stmt);
    }
    rc = sqlite3_finalize(stmt);
    return foods;
}

void database_deinit_food(Foods *foods)
{
    for (size_t i = 0; i < foods->len; i++)
    {
        free((void *)foods->items[i].name);
    }
}
