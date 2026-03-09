#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Test
{
    int a;
    int b;
    int c;
} Test;

// #define TEST(...) for (__VA_ARGS__)

void open(Test test)
{
    printf("open %i\n", test.a);
}

void close()
{
    printf("close\n");
}

#define TEST(...) for (i = (open((Test)__VA_ARGS__), 0); i < 1; i = 1, close())

int test_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int i = 0;
    TEST({.a = 10, .b = 20})
    {
        printf("here\n");
    }
    return EXIT_SUCCESS;
}
