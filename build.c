// TODO: make it as external file in stb style
// TODO: remove unsued include
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

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

static time_t
get_last_modification(const char *path)
{
    struct stat attr;
    assert(stat(path, &attr) == 0);
    return attr.st_mtim.tv_sec;
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

typedef struct {
    size_t position;
    size_t size;
    uint8_t *data;
} Arena;

#define KB(SIZE) (SIZE << 10)
#define MB(SIZE) (KB(SIZE) << 10)

static Arena
create_arena(size_t size)
{
    return (Arena){ .position = 0, .size = size, .data = malloc(size) };
}

static void
delete_arena(Arena arena)
{
    free(arena.data);
}

static void *
arena_base_alloc(Arena *arena, size_t size, size_t align)
{
    size_t shift = 0;
    size_t mod = arena->position % align;
    if (mod != 0) shift = align - mod;
    size_t position = arena->position + shift;
    assert(position >= arena->position);
    assert((position % align) == 0);
    arena->position = position + size;
    assert(arena->position <= arena->size);
    printf("arena: size: %li, from position: %li, to position: %li, used: %ld%%\n",
           size, position, arena->position,
           ((arena->position * 100) / arena->size));
    return arena->data + position;
}

#define arena_create(ARENA, TYPE) arena_base_alloc(ARENA, sizeof(TYPE), alignof(TYPE))
#define arena_alloc(ARENA, LEN, TYPE) arena_base_alloc(ARENA, LEN * sizeof(TYPE), alignof(TYPE))

typedef struct {
    char const *name;
    char *bin_path;
    int result;
} Build;

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
static Build
build(Arena *arena, const char *name, const char *src)
{
    printf("build.c: build \"%s\"\n", name);
    char *src_path = join_paths(arena, "./src", src);

    int result = mkdir("./out", 0755);
    assert(result == 0 || errno == EEXIST);
    char *bin_path = join_paths(arena, "./out", name);

    char *const argv[] = { "gcc", src_path, "-o", bin_path, NULL };
    int build_result = run(argv);
    return (Build){ .name = name, .bin_path = bin_path, .result = build_result };
}

static void
run_build(Arena *arena, Build build)
{
    printf("build.c: run \"%s\"\n", build.name);
    char *const argv[] = { build.bin_path, NULL };
    run(argv);
}

int
main()
{
    Arena arena = create_arena(KB(1));
    // TODO: Use build command
    if (rebuild()) {
        char *const argv[] = { "./build", NULL };
        run(argv);
        return 0;
    }
    Build umai = build(&arena, "umai", "main.c");
    run_build(&arena, umai);
    delete_arena(arena);
    return 0;
}
