#include <string.h>
#include <math.h>

#define AHB_LIB_IMPLEMENTATION
#define AHB_STRIP_PREFIX
#include "ahb_lib.h"

#include "raylib.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

/// begin parsing helper functions
void skip_spaces_and_breaks(String *str)
{
    while (str->data[0] == ' ' || str->data[0] == '\n') {
        printf("deleting empty space\n");
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


char *filename;

#define MAX_RENDER_LIST_SIZE 256
size_t render_list_index = 0;
RenderObject render_list[MAX_RENDER_LIST_SIZE] = {0};

#define FONT_SIZE 14

Vector2 advance_cursor(Vector2 *cursor, int x, int y)
{
    cursor->x += x;
    cursor->y += y;
    if (cursor->x > WINDOW_WIDTH) cursor->x = 0;
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

void break_line(char *text, Vector2 *cursor)
{
    Log(INFO, "text to print: %s", text);
    size_t i;
    for (i = 0; i < strlen(text); ++i) {
        // TASK(20260911-094431): find a way to not dup the text
        char *tmp = strdup(text);
        tmp[i] = '\0';
        float new_x = cursor->x + MeasureText(tmp, FONT_SIZE);
        printf("  cursor x: %.1f, new_x: %.1f\n", cursor->x, new_x);
        free(tmp);
        if (new_x >= WINDOW_WIDTH) break;
    }
    Log(INFO, "This line should be broken at index: %zu", i);
    // go back to the first space if in middle of word
    while (text[i] != ' ') {
        i -= 1;
    }
    i++; // get rid of the space for the next line

    RenderObject obj2 = {
        .cursor = *cursor,
        .text = strdup(&text[i])
    };
    Log(INFO, "text 2: %s", obj2.text);

    text[i] = '\0';
    RenderObject obj1 = {
        .cursor = *cursor,
        .text = strdup(text)
    };
    Log(INFO, "text 1: %s", obj1.text);
    Log(INFO, "cursor 1x: %f", obj1.cursor.x);

    add_to_render_list(obj1, TYPE_TEXT);
    // printf("  antes x: %f, y: %f\n", cursor->x, cursor->y);
    advance_cursor(cursor, MeasureText(&text[i], FONT_SIZE), FONT_SIZE);
    // printf("  depois x: %f, y: %f\n", cursor->x, cursor->y);

    obj2.cursor = *cursor;
    Log(INFO, "text 2: %s", obj2.text);
    Log(INFO, "cursor 2x: %f", obj2.cursor.x);

    add_to_render_list(obj2, TYPE_TEXT);
    // printf("  antes x: %f, y: %f\n", cursor->x, cursor->y);
    advance_cursor(cursor, MeasureText(text, FONT_SIZE), 0);
    // printf("  depois x: %f, y: %f\n", cursor->x, cursor->y);
}

// immediate parsing md file
void parse(char *content)
{
    Vector2 cursor = {0};

    StringList lines = {0};
    split(content, '\n', &lines);
    // Log(INFO, "Number of lines in md file: %zu", lines.count);
    for (size_t i = 0; i < lines.count - 1; ++i) {
        String line = lines.data[i];
        skip_spaces_and_breaks(&line);
        // Log(INFO, "Parsing line %zu... Content: %s", i+1, line.data);
        while (line.count > 0) {
            char c = get_char(&line);
            if (c == '%' && peek_next_char(&line) == '%') {
                // comment
                break;
            }
            if (c == '!') {
                // image
                c = pop_char(&line);
                Image image = get_image_path(&line);
                Texture2D texture = LoadTextureFromImage(image);
                RenderObject obj = {
                    .cursor = cursor,
                    .texture = texture
                };
                add_to_render_list(obj, TYPE_TEXTURE);
                advance_cursor(&cursor, texture.width, 0);
                free(image.data);
                break;
            }
            else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '1')) {
                // text
                // text rendering always begin in a new line on the cursor
                RenderObject last = render_list[render_list_index - 1];
                static_assert(TYPE_COUNT == 2 && "Missing 'RenderObjectType' in switch case");
                switch (last.type) {
                // TASK(20260911-094659): get the maximum height in all objects of the previous line
                case TYPE_TEXTURE: cursor.y += last.texture.height; break; 
                case TYPE_TEXT: cursor.y += FONT_SIZE; break;
                default:
                    ABORT("Unknown render object type");
                }
                cursor.x = 0;

                float text_size_in_px = MeasureText(line.data, FONT_SIZE);
                bool need_break = cursor.x + text_size_in_px > WINDOW_WIDTH; 
                if (need_break) {
                    break_line(line.data, &cursor);
                }
                break;
            }
            else {
                Log(WARN, "No markdown tag that starts with '%c' is defined in this md viewer", c);
                continue;
            }
        }
    }
    split_free(&lines);
}

void draw_object(RenderObject obj, bool debug)
{
    Vector2 cursor_pos = (Vector2){obj.cursor.x, obj.cursor.y};


    static_assert(TYPE_COUNT == 2 && "Missing 'RenderObjectType' in switch case");
    switch (obj.type) {
    case TYPE_TEXTURE:
        DrawTextureEx(obj.texture, cursor_pos, 0.0, 1.0, WHITE);
        if (debug) DrawRectangleLines(cursor_pos.x, cursor_pos.y, obj.texture.width, obj.texture.height, MAGENTA);
        break;
    case TYPE_TEXT:
        DrawText(obj.text, cursor_pos.x, cursor_pos.y, FONT_SIZE, WHITE);
        if (debug) DrawRectangleLines(cursor_pos.x, cursor_pos.y, MeasureText(obj.text, FONT_SIZE), FONT_SIZE, MAGENTA);
        break;
    default:
        ABORT("Unknown render object type");
    }
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

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "MD Viewer");
    SetTargetFPS(30);

    BeginDrawing();
    ClearBackground(GetColor(0x181818FF));
    EndDrawing();
    
    parse(content);

    // Log(INFO, "Number of render objects: %zu", render_list_index);
    bool draw_debug = false;
    while (!WindowShouldClose()) {
        BeginDrawing();

        if (IsKeyPressed(KEY_F5)) {
            // TASK(20260911-094144): reset state of application when F5 is pressed
            content = read_entire_file(filename);
            parse(content);
        }
        else if (IsKeyPressed(KEY_Q)) {
            break;
        }

        for (size_t i = 0; i <= render_list_index; ++i) {
            draw_object(render_list[i], draw_debug);
        }

        EndDrawing();
    }

    free(content);

    return 0;
}
