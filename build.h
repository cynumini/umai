#include <stddef.h>
#include <stdint.h>

typedef struct
{
    size_t position;
    size_t next_position;
    size_t size;
    uint8_t *data;
} Arena;

typedef struct
{
    Arena arena;
} BuildState;

void
build_begin();

void
build_end();

#define dynamic_array_append(ARENA, ARRAY, ITEM) do                                               \
{                                                                                                 \
    if ((ARRAY)->len >= (ARRAY)->capacity)                                                        \
    {                                                                                             \
        if ((ARRAY)->capacity == 0)                                                               \
        {                                                                                         \
            (ARRAY)->capacity = 1;                                                                \
            (ARRAY)->data = arena_alloc(ARENA, 1, typeof((ARRAY)->data[0]));                      \
        }                                                                                         \
        else                                                                                      \
        {                                                                                         \
            size_t old_capacity = (ARRAY)->capacity;                                              \
            (ARRAY)->capacity *= 2;                                                               \
            (ARRAY)->data = arena_realloc(ARENA, (ARRAY)->data, old_capacity, (ARRAY)->capacity); \
        }                                                                                         \
    }                                                                                             \
    (ARRAY)->data[(ARRAY)->len] = ITEM;                                                           \
    (ARRAY)->len++;                                                                               \
} while(0)

typedef struct
{
    char **data;
    size_t len;
    size_t capacity;
} StringsArray;

typedef struct
{
    char const* name;
    char *src_path;
    char *bin_path;
    StringsArray include_dirs;
    StringsArray lib_dirs;
    StringsArray libs;
} Build;

Build
create_build(const char *name, const char *src);

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
char *execvpe(const char *file, char *const argv[], char *const envp[]);
extern char **environ;
#include <unistd.h>
#include <string.h>

static Arena
create_arena(size_t size)
{
    return (Arena){ .next_position = 0, .size = size, .data = malloc(size) };
}

#define KB(SIZE) (SIZE << 10)
#define MB(SIZE) (KB(SIZE) << 10)

static time_t
get_last_modification(const char *path)
{
    struct stat attr;
    assert(stat(path, &attr) == 0);
    return attr.st_mtim.tv_sec;
}

static int
run(char *const argv[])
{
    size_t index = 0;
    char const *arg = argv[index];
    printf("run:");
    while(arg != NULL)
    {
        printf(" %s", arg);
        index++;
        arg = argv[index];
    }
    printf("\n");
    pid_t pid = fork();
    if (pid == 0)
    {
        assert(execvp(argv[0], argv) == 0);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        return status;
    }
}

static bool
rebuild()
{
    time_t bin_time = get_last_modification("build");
    time_t src_time = get_last_modification("build.c");
    if (bin_time < src_time)
    {
        printf("build.c: self rebuild\n");
        char *const argv[] = { "gcc", "./build.c", "-o", "build", NULL };
        if (run(argv) != 0)
        {
            printf("build.c: can't self rebuild\n");
            return false;
        }
        return true;
    }
    return false;
}

void
build_begin()
{
    build_arena = create_arena(KB(1));
    // TODO: Use build command
    if (rebuild()) {
        char *const argv[] = { "./build", NULL };
        run(argv);
        exit(0);
    }
}

static void
delete_arena(Arena arena)
{
    free(arena.data);
}

void
build_end()
{
    delete_arena(build_arena);
}

// TODO: fill data with zero by default
static void *
arena_base_alloc(Arena *arena, size_t size, size_t align)
{
    size_t shift = 0;
    size_t mod = arena->next_position % align;
    if (mod != 0) shift = align - mod;
    arena->position =  arena->next_position + shift;
    assert(arena->position >= arena->next_position);
    assert((arena->position % align) == 0);
    arena->next_position = arena->position + size;
    assert(arena->next_position <= arena->size);
    printf("arena.alloc: size: %li, from position: %li, to position: %li, used: %ld%%\n",
           size, arena->position, arena->next_position,
           ((arena->next_position * 100) / arena->size));
    return arena->data + arena->position;
}

#define arena_create(ARENA, TYPE) arena_base_alloc(ARENA, sizeof(TYPE), alignof(TYPE))
#define arena_alloc(ARENA, LEN, TYPE) arena_base_alloc(ARENA, LEN * sizeof(TYPE), alignof(TYPE))

static void *
arena_base_realloc(Arena *arena, void *ptr, size_t old_size, size_t new_size, size_t align)
{
    void *result = ptr;
    if (ptr == arena->data + arena->position)
    {
        arena->next_position = arena->position + new_size;
        assert(arena->next_position <= arena->size);
        printf("arena.realloc: size: %li, from position: %li, to position: %li, used: %ld%%\n",
           new_size, arena->position, arena->next_position,
           ((arena->next_position * 100) / arena->size));
    }
    else
    {
        result = arena_base_alloc(arena, new_size, align);
        memcpy(result, ptr, new_size);
    }
    return result;
}


#define arena_realloc(ARENA, PTR, OLD_LEN, NEW_LEN) arena_base_realloc(ARENA, PTR, OLD_LEN * sizeof(typeof(PTR[0])), NEW_LEN * sizeof(typeof(PTR[0])), alignof(typeof(PTR[0])))

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

    char *result = arena_alloc(arena, result_len, char);
    if (is_a_have_slash)
    {
        sprintf(result, "%s%s", a, b);
    } else
    {
        sprintf(result, "%s/%s", a, b);
    }
    return result;
}

// TODO: add valgrind
// TODO: add warning
// TODO: add warning as error
// TODO: add type conversion warning
// TODO: add multiple files
// TODO: add caching
// TODO: add custom gcc arguments
// TODO: add linking
// TODO: add headers
Build
create_build(const char *name, const char *src)
{
    printf("build.c: build \"%s\"\n", name);
    char *src_path = join_paths(&build_arena, "./src", src);

    int result = mkdir("./out", 0755);
    assert(result == 0 || errno == EEXIST);
    char *bin_path = join_paths(&build_arena, "./out", name);

    return (Build){ .name = name, .src_path = src_path, .bin_path = bin_path };
}

static ptrdiff_t
rfind(char *str, char c)
{
    for (size_t i = strlen(str) - 1; i >= 0; i--)
    {
        if (str[i] == c)
        {
            return i;
        }
    }
    return -1;
}

void
add_include_dir(Build *build, char *include_path)
{
    dynamic_array_append(&build_arena, &build->include_dirs, include_path);
}

void
add_lib(Build *build, char *lib_path)
{
    ptrdiff_t result = rfind(lib_path, '/');
    assert(result > 0);
    char *lib_dir = arena_alloc(&build_arena, result + 1, char);
    memcpy(lib_dir, lib_path, result);
    char *lib = arena_alloc(&build_arena, strlen(lib_path) - result + 1 - 7, char);
    memcpy(lib, lib_path + result + 4, strlen(lib_path) - result + 1 - 7);
    printf("lib: %s\n", lib);

    dynamic_array_append(&build_arena, &build->lib_dirs, lib_dir);
    add_system_lib(build, lib);
}

void
add_system_lib(Build *build, char *lib)
{
    dynamic_array_append(&build_arena, &build->libs, lib);
}

bool
compile_build(Build build)
{
    StringsArray args = {};
    dynamic_array_append(&build_arena, &args, "gcc");
    dynamic_array_append(&build_arena, &args, build.src_path);

    for (size_t i = 0; i < build.include_dirs.len; i++)
    {
        char *item = build.include_dirs.data[i];
        size_t len = strlen("-I") + strlen(item) + 1;
        char *path = arena_alloc(&build_arena, len, char);
        sprintf(path, "%s%s", "-I", item);
        dynamic_array_append(&build_arena, &args, path);
    }

    for (size_t i = 0; i < build.lib_dirs.len; i++)
    {
        char *item = build.lib_dirs.data[i];
        size_t len = strlen("-L") + strlen(item) + 1;
        char *path = arena_alloc(&build_arena, len, char);
        sprintf(path, "%s%s", "-L", item);
        dynamic_array_append(&build_arena, &args, path);
    }

    for (size_t i = 0; i < build.libs.len; i++)
    {
        char *item = build.libs.data[i];
        size_t len = strlen("-l") + strlen(item) + 1;
        char *path = arena_alloc(&build_arena, len, char);
        sprintf(path, "%s%s", "-l", item);
        dynamic_array_append(&build_arena, &args, path);
    }

    dynamic_array_append(&build_arena, &args, "-o");
    dynamic_array_append(&build_arena, &args, build.bin_path);
    dynamic_array_append(&build_arena, &args, NULL);
    return run(args.data) == 0;
}

// TODO: measure run time
// TODO: gf2, lldb and gdb support
void
run_build(Build build)
{
    printf("build.c: run \"%s\"\n", build.name);
    char *const argv[] = { build.bin_path, NULL };
    run(argv);
}
#endif // SKN_BUILD_IMPLEMENTATION
