typedef struct TableData
{
    void *data;
    size_t (*get_row_count)(void *data);
    size_t (*get_column_count)(void *data);
    void (*get_cell)(void *data, size_t row, size_t column, char *buffer);
} TableData;

struct Table
{
    OldNode node;
    TableData data;
    int selected_column;
    // Using inside on_row_selected
    void *user_data;
    void (*on_row_selected)(Table *self, int index, void *user_data);
};

struct TextInput
{
    OldNode node;
    char text[256];
    // Using inside on_text_changed
    void *user_data;
    void (*on_text_changed)(TextInput *self, void *user_data);
};
