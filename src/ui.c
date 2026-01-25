#include "ui.h"

#include "SKN/arena.h"
#include <stdio.h>
#include <string.h>

Context context_create(Arena *arena)
{
    return (Context){
        .arena = arena,
    };
}

void node_open(Context *ctx, Node options)
{
    Node *node = ARENA_PUSH_STRUCT(ctx->arena, Node);
    *node = options;
    if (ctx->current == NULL)
    {
        ctx->current = node;
    }
    else
    {
        node->parent = ctx->current;
        node_add_child(ctx, ctx->current, node);
    }
}

void node_close(Context *context)
{
    if (context->current->parent != NULL)
    {
        context->current = context->current->parent;
    }
}

void node_add_child(Context *ctx, Node *self, Node *child)
{
    if (self->children == NULL)
    {
        self->children = ARENA_PUSH_STRUCT_ZERO(ctx->arena, NodeChildren);
        self->children->value = child;
    }
    else
    {
        NodeChildren *next_child = self->children;
        while (next_child->next != NULL)
        {
            next_child = next_child->next;
        }
        next_child->next = ARENA_PUSH_STRUCT_ZERO(ctx->arena, NodeChildren);
        next_child->next->value = child;
    }
}

void node_print(Node *node, int level)
{
    for (int i = 0; i < level; i++)
    {
        printf("\t");
    }
    printf("id: %s\n", node->id);
    NodeChildren *current = node->children;
    while (current != NULL)
    {
        node_print(current->value, level + 1);
        current = current->next;
    }
}
