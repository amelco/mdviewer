#include <stdio.h>
#include <string.h>

#define AHB_LIB_IMPLEMENTATION
#define AHB_STRIP_PREFIX
#include "ahb_lib.h"

#include "raylib.h"

/// begin parsing helper functions
void skip_spaces(String *str)
{
    if (str->data[0] == ' ') {
        str->data += 1;
        str->count -= 1;
    }
}

char peek_next_char(String *str)
{
    if (str->count <= 0) {
        ABORT("There's no next char to peek");
    }
    return str->data[1];
}

char get_char(String *str)
{
    return str->data[0];
}

char pop_char(String *str)
{
    char c = str->data[0];
    str->data += 1;
    str->count -= 1;
    return c;
}

bool skip_until_char(char expected, String *str, char **out)
{
    if (str->count == 0) return false;

    String res = {0};
    res.data = str->data;

    char c = pop_char(str); 
    while (c != expected) {
        c = pop_char(str);
        res.count += 1;
        if (str->count == 0 && c != expected) {
            return false;
        }
    }
    *out = ahb_String_to_cstr(res);
    assert(out != NULL);
    return true;
}
//// end parsing functions

Image get_image_path(String *line)
{
    Log(INFO, "Loading an image");
    if (get_char(line) != '[') ABORT("Expected '['");
    char c = pop_char(line);
    char *alt = NULL;
    if (!skip_until_char(']', line, &alt)) ABORT("Expected ']'");
    c = pop_char(line);
    if (c != '(') ABORT("Expected '('");
    char *image_path = NULL;
    if (!skip_until_char(')', line, &image_path)) ABORT("Expected ')'");

    Image image = LoadImage(image_path);
    if (!IsImageValid(image)) ABORT("Image is not valid");
    return image;
}

typedef enum {
    TYPE_TEXTURE,
    TYPE_TEXT
} RenderObjectType;

typedef struct {
    Vector2 cursor;
    union {
        Texture2D texture;
        char* text;
    };
    RenderObjectType type;
} RenderObject;


char *filename;

#define MAX_RENDER_LIST_SIZE 256
size_t render_list_index = 0;
RenderObject render_list[MAX_RENDER_LIST_SIZE] = {0};

Vector2 advance_cursor(Vector2 *cursor, int x, int y)
{
    cursor->x += x;
    cursor->y += y;
    // TODO: check if y is at end of renderer width to render into another line
    return *cursor;
}

void add_to_render_list(RenderObject obj, RenderObjectType type)
{
    if (render_list_index >= MAX_RENDER_LIST_SIZE) {
        Log(WARN, "Maximum size of render list achieved. Item not added.");
        return;
    }
    render_list[render_list_index] = obj;
    render_list[render_list_index].type = type;
    render_list_index += 1;
}

// immediate parsing md file
void parse(char *content)
{
    Vector2 cursor = {0};

    StringList lines = {0};
    split(content, '\n', &lines);
    for (size_t i = 0; i < lines.count; ++i) {
        String line = lines.data[i];
        while (line.count > 0) {
            skip_spaces(&line);
            char c = pop_char(&line);
            if (c == '!') {
                Image image = get_image_path(&line);
                Texture2D texture = LoadTextureFromImage(image);
                RenderObject obj = {
                    .cursor = cursor,
                    .texture = texture
                };
                add_to_render_list(obj, TYPE_TEXTURE);
                advance_cursor(&cursor, texture.width, 0);
                free(image.data);
            }
        }
    }
    split_free(&lines);
}

int main(int argc, char **argv) {
    args_init(argc, argv);
    if (argc < 2) {
        Log(INFO, "Usage: %s <filename.md>\n", argv[0]);
        exit(0);
    }
    filename = args_next();
    if(!file_exits(filename)) {
        Log(ERROR, "File '%s' does not exist", filename);
        exit(1);
    }

    char *content = read_entire_file(filename);

    InitWindow(800, 600, "MD Viewer");
    SetTargetFPS(30);
    
    parse(content);

    while (!WindowShouldClose()) {
        BeginDrawing();

        for (size_t i = 0; i <= render_list_index; ++i) {
            Vector2 cursor_pos = (Vector2){render_list[i].cursor.x, render_list[i].cursor.y};
            switch (render_list[i].type) {
            case TYPE_TEXTURE:
                DrawTextureEx(render_list[i].texture, cursor_pos, 0.0, 1.0, WHITE);
                break;
            case TYPE_TEXT:
                TODO("TYPE_TEXT");
                break;
            default:
                ABORT("Unknown render object type");
            }
        }

        EndDrawing();
    }

    free(content);

    return 0;
}
