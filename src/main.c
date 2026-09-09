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

typedef struct {
    Image *data;
    size_t count;
    size_t capacity;
} ImageList;

typedef struct {
    Texture2D *data;
    size_t count;
    size_t capacity;
} TexturesList;

typedef struct {
    float x;
    float y;
} Cursor;

char *filename;

// TODO: make a "cache" so the images are not loaded at every frame 
//       OR
//       do not make an immediate rendering (along with the parsing)
void render(char *content)
{
    Cursor cursor = {0};
    BeginDrawing();

    // immediate parsing md file
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
                DrawTextureEx(texture, (Vector2){cursor.x, cursor.y}, 0.0, 1.0, WHITE);
                cursor.x += texture.width;
                free(image.data);
            }
        }
    }
    split_free(&lines);
    EndDrawing();
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
    SetTargetFPS(15);
    
    render(content);

    while (!WindowShouldClose()) {
        render(content);
    }

    free(content);

    return 0;
}
