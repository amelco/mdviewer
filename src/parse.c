#define AHB_STRIP_PREFIX
#include "ahb_lib.h"

#include "raylib.h"

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
