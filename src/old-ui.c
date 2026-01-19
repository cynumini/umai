#include "old-ui.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

// NodeLabel
static void node_label_draw(Node *node)
{
    NodeLabel *self = (NodeLabel *)node;
    DrawText(self->text, node->actual_position.x, node->actual_position.y, 20, BLACK);
}

NodeLabel node_label_init(const char *name)
{
    return (NodeLabel){
        {
            {size_fixed(MeasureText(name, 20)), size_fixed(20)},
            {0, 0},
            {0, 0},
            {0},
            node_label_draw,
        },
        name,
    };
}

// NodeButton
static void node_button_draw(Node *node)
{
    NodeButton *self = (NodeButton *)node;
    Rectangle rect = {
        node->actual_position.x,
        node->actual_position.y,
        node->sizing.width.value,
        node->sizing.height.value,
    };
    DrawRectangleLinesEx(rect, 1, BLACK);
    DrawText(self->text, rect.x + 2, rect.y + 2, 20, BLACK);
}

NodeButton node_button_init(const char *name)
{
    return (NodeButton){
        {
            {size_fixed(MeasureText(name, 20) + 4), size_fixed(20 + 4)},
            {0, 0},
            {0, 0},
            {0},
            node_button_draw,
        },
        name,
    };
}

// NodeRectangle

// NodeContainer
static void node_container_draw(Node *node)
{
    NodeContainer *self = (NodeContainer *)node;

    DrawRectangle(node->position.x, node->position.y, node->sizing.width.value, node->sizing.height.value, RED);
    switch (self->type)
    {
    case LAYOUT_DIRECTION_TOP_TO_BOTTOM: {
        int top_offset = node->padding.top;
        for (size_t i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            Vector2 child_position = Vector2Add(node->actual_position, child->position);
            child_position.x += node->padding.left;
            child_position.y += top_offset;
            child->actual_position = child_position;
            child->draw(child);
            top_offset += child->sizing.height.value + self->child_gap;
        }
        break;
    }
    case LAYOUT_DIRECTION_LEFT_TO_RIGHT: {
        int left_offset = node->padding.left;
        for (size_t i = 0; i < self->children.len; i++)
        {
            Node *child = self->children.items[i];
            Vector2 child_position = Vector2Add(node->actual_position, child->position);
            child_position.x += left_offset;
            child_position.y += node->padding.top;
            child->actual_position = child_position;
            child->draw(child);
            left_offset += child->sizing.width.value + self->child_gap;
        }
        break;
    }
    }
}

NodeContainer node_container_init(Sizing sizing, NodeContainerType type, Padding padding, int child_gap)
{
    return (NodeContainer){
        {sizing, {0, 0}, {0, 0}, padding, node_container_draw},
        type,
        child_gap,
        {0},
    };
}

void node_container_deinit(NodeContainer *self)
{
    DYNAMIC_ARRAY_DEINIT(&self->children);
}

void node_container_add_child(NodeContainer *self, Node *child)
{
    DYNAMIC_ARRAY_ADD(&self->children, child);
}
