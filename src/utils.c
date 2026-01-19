#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

bool utils_path_exist(const char *path)
{
    struct stat path_stat;
    if (stat(path, &path_stat) == 0)
        return true;
    return false;
}

// Reads the entire file and returns its contents as text.
// The caller must free the returned value.
char *read_text(const char *path)
{
    FILE *file = fopen(path, "r");
    assert(file);
    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size != -1);
    rewind(file);
    char *buffer = malloc(size + 1);
    assert(buffer);
    assert(fread(buffer, 1, size, file) == (size_t)size);
    assert(fclose(file) == 0);
    buffer[size] = '\0';
    return buffer;
}

void __unreachable(const char *file, const int line, const char *func)
{
    fprintf(stderr, "%s:%d: %s: Reached unreachable code\n.", file, line, func);
    abort();
}

#define MAX(TYPE)                                                                                                      \
    TYPE max_##TYPE(TYPE a, TYPE b)                                                                                    \
    {                                                                                                                  \
        return a > b ? a : b;                                                                                          \
    }
#define MIN(TYPE)                                                                                                      \
    TYPE min_##TYPE(TYPE a, TYPE b)                                                                                    \
    {                                                                                                                  \
        return a < b ? a : b;                                                                                          \
    }

MAX(int)
MIN(int)
MAX(float)
MIN(float)
