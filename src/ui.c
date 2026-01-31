#include "ui.h"
#include <stdio.h>

static UI ui = {0};

void ui_init(void)
{
    ui.arena = arena_create(MB(1));
}

void ui_deinit(void)
{
    arena_destroy(&ui.arena);
}

void node_add_child(Node *self, Node *child)
{
    self->add_child(self, child);
}

void container_open(ContainerOptions options)
{
    Contrainer *self = ARENA_PUSH_STRUCT_ZERO(&ui.arena, Contrainer);
    self->node.id = options.id;
    if (ui.current == NULL)
    {
        ui.current = &self->node;
    }
    else
    {
        node_add_child(ui.current, &self->node);
    }
    ui.current = &self->node;
}

void container_close(void)
{
    printf("container_close\n");
}
