#ifndef RENDER_H_
#define RENDER_H_

#include "raylib.h"
#include "stdlib.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define FONT_SIZE 14

typedef enum {
    TYPE_TEXTURE,
    TYPE_TEXT,
    TYPE_COUNT
} RenderObjectType;

typedef struct {
    Vector2 cursor;
    union {
        Texture2D texture;
        char* text;
    };
    RenderObjectType type;
} RenderObject;

#define MAX_RENDER_LIST_SIZE 256
// size_t render_list_index = 0;
// RenderObject render_list[MAX_RENDER_LIST_SIZE] = {0};
extern size_t render_list_index;
extern RenderObject render_list[MAX_RENDER_LIST_SIZE];

void draw_object(RenderObject obj, bool debug);
void parse_to_render_object_list(char *content);
void put_cursor_on_new_line(Vector2 *cursor);
bool line_needs_break(char *line, Vector2 cursor);
void break_line(char *text, Vector2 *cursor);
void add_to_render_list(RenderObject obj, RenderObjectType type);

#endif // RENDER_H_
