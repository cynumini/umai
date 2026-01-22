#include "ui.h"

#include "arena.h"

void node_open(Context *ctx, Node node)
{
    Node *self = ARENA_PUSH_STRUCT(ctx->arena, Node);
    *self = node;
    if (self->parent == NULL)
    {
        self->parent = ctx->current;
    }
    node_add_child(ctx, ctx->current, self);
    ctx->current = self;
}

void node_close(Context *context)
{
    context->current = context->current->parent;
}

void node_add_child(Context *ctx, Node *self, Node *child)
{
    list_node_add(ctx->arena, self->children, child);
}
