#ifndef SKN_BUILD_H
#define SKN_BUILD_H

#include "da.h"
#define SKN_ARENA_IMPLEMENTATION
#include "arena.h"

void
begin_build_base(int argc, char **argv, char const *file);
#define begin_build(ARGC, ARGV) begin_build_base(ARGC, ARGV, __FILE__)

DEFINE_DA(Strings, char *);
DEFINE_DA(Cmd, char *);

int
run_cmd(Cmd cmd);

typedef struct
{
    char const* name;
    char *src_path;
    char *bin_path;
    Strings include_dirs;
    Strings lib_dirs;
    Strings libs;
    bool debug;
    bool warning;
    bool warning_error;
} Build;

Build
create_build_base(char const *name, char const *src, char const *src_dir, char const *out_dir);
#define create_build(NAME, SRC) create_build_base(NAME, SRC, "./src", "./out")

void
add_include_dir(Build *build, char *include_path);

void
add_lib(Build *build, char *lib_path);

void
add_system_lib(Build *build, char *lib_path);

bool
compile_build(Build build);

void
run_build(Build build);

#ifdef SKN_BUILD_IMPLEMENTATION
static Arena build_arena;

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

static time_t
get_last_modification(const char *path)
{
    struct stat attr;
    assert(stat(path, &attr) == 0);
    return attr.st_mtim.tv_sec;
}

int
run_cmd(Cmd cmd)
{
    printf("run_cmd:");
    assert(cmd.len > 1);
    for (size_t i = 0; i < (cmd.len - 1); i++) printf(" %s", cmd.data[i]);
    printf("\n");
    pid_t pid = fork();
    assert(pid != -1);
    if (pid == 0)
    {
        execvp(cmd.data[0], cmd.data);
        fprintf(stderr, "run_cmd: failed to run subprocess\n");
        _exit(127);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        return status;
    }
}

static bool
rfind(char const *str, char c, size_t *result)
{
    for (size_t i = strlen(str) - 1; i > 0; i--)
    {
        if (str[i] == c)
        {
            *result = i;
            return true;
        }
    }
    return false;
}

static char const *
basename(char const *path)
{
    size_t position = 0;
    if (rfind(path, '/', &position)) {
        assert(position + 1 < strlen(path));
        return path + position + 1;
    }
    return path;

}

static void
end_build()
{
    delete_arena(build_arena);
}

static void
cmd_append(Cmd *cmd, char *string)
{
    if (cmd->len == 0)
    {
        da_append(&build_arena, cmd, string);
        da_append(&build_arena, cmd, NULL);
        return;
    }
    cmd->data[cmd->len - 1] = string;
    da_append(&build_arena, cmd, NULL);
}

void
begin_build_base([[maybe_unused]]int argc, char **argv, char const *src_path)
{
    build_arena = alloc_arena(KB(1));
    atexit(end_build);

    char *bin_path = argv[0];

    time_t bin_time = get_last_modification(bin_path);
    time_t src_time = get_last_modification(src_path);

    if (bin_time < src_time)
    {
        Build self = create_build_base(basename(bin_path), basename(src_path), "./", "./");
        self.debug = true;
        self.warning = true;
        if (compile_build(self)) {
            Cmd cmd = {};
            cmd_append(&cmd, bin_path);
            run_cmd(cmd);
            exit(0);
        }
        printf("rebuild: can't rebuild oneself\n");
        exit(1);
    }
}

static char *
join_paths(Arena *arena, char const *a, char const *b)
{
    size_t a_len = strlen(a);
    assert(a_len >= 1);
    bool is_a_have_slash = false;
    if (a[a_len - 1] == '/') is_a_have_slash = true;
    size_t b_len = strlen(b);
    assert(b_len >= 1);
    size_t result_len = a_len + b_len + 1;
    if (!is_a_have_slash) result_len++;
    char *result = arena_alloc(arena, char, result_len);
    strcat(result, a);
    if (!is_a_have_slash) strcat(result, "/");
    strcat(result, b);
    return result;
}

// TODO: add multiple files
// TODO: add caching
// TODO: add custom gcc arguments
Build
create_build_base(const char *name, const char *src, const char *src_dir, const char *out_dir)
{
    printf("build: \"%s\"\n", name);
    char *src_path = join_paths(&build_arena, src_dir, src);
    char *bin_path = join_paths(&build_arena, out_dir, name);

    int result = mkdir(out_dir, 0755);
    assert(result == 0 || errno == EEXIST);

    return (Build){ .name = name, .src_path = src_path, .bin_path = bin_path };
}

void
add_include_dir(Build *build, char *include_path)
{
    da_append(&build_arena, &build->include_dirs, include_path);
}

static char *
substr(char const *src, size_t start, size_t end)
{
    assert(start < end);
    assert(end < strlen(src));
    size_t len = end - start;
    char *result = arena_alloc(&build_arena, char, len + 1);
    memcpy(result, src + start, len);
    return result;
}

static char*
dirname(char const *path)
{
    size_t pos = 0;
    rfind(path, '/', &pos);
    assert(pos > 0);
    return substr(path, 0, pos);
}

void
add_lib(Build *build, char *lib_path)
{
    da_append(&build_arena, &build->lib_dirs, dirname(lib_path));
    char const *file_name = basename(lib_path);
    add_system_lib(build, substr(file_name, 3, strlen(file_name) - 2));
}

void
add_system_lib(Build *build, char *lib)
{
    da_append(&build_arena, &build->libs, lib);
}

char *
joinstr(char const *a, char const *b)
{
    size_t len = strlen(a) + strlen(b) + 1;
    char *result = arena_alloc(&build_arena, char, len);
    strcat(result, a);
    strcat(result, b);
    return result;
}

bool
compile_build(Build build)
{
    Cmd args = {};
    cmd_append(&args, "gcc");
    cmd_append(&args, build.src_path);
    foreach (char *, item, &build.include_dirs) cmd_append(&args, joinstr("-I", *item));
    foreach (char *, item, &build.lib_dirs)     cmd_append(&args, joinstr("-L", *item));
    foreach (char *, item, &build.libs)         cmd_append(&args, joinstr("-l", *item));
    if (build.debug) cmd_append(&args, "-g");
    if (build.warning) {
        cmd_append(&args, "-Wall");
        cmd_append(&args, "-Wextra");
        cmd_append(&args, "-Wpedantic");
        cmd_append(&args, "-Wconversion");
    }
    if (build.warning_error) cmd_append(&args, "-Werror");
    cmd_append(&args, "-o");
    cmd_append(&args, build.bin_path);
    return run_cmd(args) == 0;
}

// TODO: measure run time
// TODO: gf2, lldb and gdb support
// TODO: add valgrind
// TODO: pass custom args
void
run_build(Build build)
{
    printf("build.c: run \"%s\"\n", build.name);
    Cmd cmd = {};
    cmd_append(&cmd, build.bin_path);
    run_cmd(cmd);
}
#endif // SKN_BUILD_IMPLEMENTATION

#endif // SKN_BUILD_H
