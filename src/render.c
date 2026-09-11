#define AHB_STRIP_PREFIX
#include <string.h>
#include "ahb_lib.h"
#include "render.h"
#include "parse.h"

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
    cursor->x += MeasureText(&text[i], FONT_SIZE);
    cursor->y += FONT_SIZE;

    obj2.cursor = *cursor;
    Log(INFO, "text 2: %s", obj2.text);
    Log(INFO, "cursor 2x: %f", obj2.cursor.x);

    add_to_render_list(obj2, TYPE_TEXT);
    cursor->x += MeasureText(text, FONT_SIZE);
    cursor->y += 0;
}

bool line_needs_break(char *line, Vector2 cursor)
{
    float text_size_in_px = MeasureText(line, FONT_SIZE);
    return cursor.x + text_size_in_px > WINDOW_WIDTH; 
}

void put_cursor_on_new_line(Vector2 *cursor)
{
    RenderObject last = render_list[render_list_index - 1];
    static_assert(TYPE_COUNT == 2, "Missing 'RenderObjectType' in switch case");
    switch (last.type) {
    // TASK(20260911-094659): get the maximum height in all objects of the previous line
    case TYPE_TEXTURE: cursor->y += last.texture.height; break; 
    case TYPE_TEXT: cursor->y += FONT_SIZE; break;
    default:
        ABORT("Unknown render object type");
    }
    cursor->x = 0;
}

// immediate parsing md file
void parse_to_render_object_list(char *content)
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
                cursor.x += texture.width;
                cursor.y += 0;
                free(image.data);
                break;
            }
            else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '1')) {
                // text rendering always begin in a new line on the cursor
                put_cursor_on_new_line(&cursor);

                if (line_needs_break(line.data, cursor)) {
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


    static_assert(TYPE_COUNT == 2, "Missing 'RenderObjectType' in switch case");
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
