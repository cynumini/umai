#include "utils.h"
#include <sys/stat.h>

bool utils_path_exist(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) == 0) return true;
    return false;
}
