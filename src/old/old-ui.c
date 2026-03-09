static void table_calc_fit_width(OldNode *node)
{
    const Table *self = (const Table *)node;
    void *data = self->data.data;
    const size_t row_count = self->data.get_row_count(data);
    const size_t column_count = self->data.get_column_count(data);
    node->rect.width = column_count * 2 + (column_count + 1);
    for (size_t i = 0; i < column_count; i++)
    {
        float max_column_width = 0;
        for (size_t j = 0; j < row_count; j++)
        {
            char buffer[256] = {0};
            self->data.get_cell(data, j, i, buffer);
            max_column_width = max_float(MeasureText(buffer, 20), max_column_width);
        }
        node->rect.width += max_column_width;
    }
}

static void table_calc_fit_height(OldNode *node)
{
    const Table *self = (const Table *)node;
    void *data = self->data.data;
    const size_t row_count = self->data.get_row_count(data);
    node->rect.height = row_count * 2 + (row_count + 1);
    node->rect.height += row_count * 20;
}

static void table_draw(OldNode const *node)
{
    const Table *self = (const Table *)node;
    void *data = self->data.data;
    const size_t row_count = self->data.get_row_count(data);
    const size_t column_count = self->data.get_column_count(data);

    if (self->selected_column != -1)
    {
        DrawRectangleRec(
            (Rectangle){
                node->rect.x,
                node->rect.y + self->selected_column * 23,
                node->rect.width,
                23,
            },
            BLUE);
    }

    int x_offset = node->rect.x + 2;

    for (size_t i = 0; i < column_count; i++)
    {
        float max_column_width = 0;
        int y_offset = node->rect.y + 2;
        for (size_t j = 0; j < row_count; j++)
        {
            if (i == 0)
            {
                float y = y_offset + 22;
                DrawLine(node->rect.x, y, node->rect.x + node->rect.width, y, BLACK);
            }
            char buffer[256] = {0};
            self->data.get_cell(data, j, i, buffer);
            DrawText(buffer, x_offset, y_offset, 20, BLACK);
            y_offset += 23;
            max_column_width = max_float(MeasureText(buffer, 20), max_column_width);
        }
        float x = x_offset + max_column_width + 2;
        DrawLine(x, node->rect.y, x, node->rect.y + node->rect.height, BLACK);
        x_offset += max_column_width + 3;
    }

    DrawRectangleLinesEx(node->rect, 1, BLACK);
}

static void table_update(OldNode *node)
{
    Table *self = (Table *)node;
    (void)self;

    Vector2 mouse_position = GetMousePosition();
    Vector2 relative = Vector2Subtract(mouse_position, (Vector2){node->rect.x, node->rect.y});
    bool is_mouse_over = CheckCollisionPointRec(mouse_position, node->rect);
    if (is_mouse_over)
    {
        if (IsKeyReleased(KEY_ESCAPE))
        {
            self->selected_column = -1;
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            self->selected_column = (int)relative.y / 23;
            if (self->on_row_selected != NULL)
            {
                self->on_row_selected(self, self->selected_column, self->user_data);
            }
        }
    }
}

Table table_init(TableData data)
{
    return (Table){.node =
                       {
                           .calc_fit_width = table_calc_fit_width,
                           .calc_fit_height = table_calc_fit_height,
                           .update = table_update,
                           .draw = table_draw,
                       },
                   .data = data,
                   .selected_column = -1};
}

void table_deinit(Table *self)
{
    (void)self;
}

// TextInput
static void text_input_calc_fit_width(OldNode *node)
{
    (void)node;
}

TextInput text_input_init(void)
{
    return (TextInput){.node =
                           {
                               .calc_fit_width = text_input_calc_fit_width,
                               .draw = NULL,
                           },
                       .text = {0}};
}

void text_input_deinit(TextInput *self)
{
    (void)self;
}
