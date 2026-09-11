#include <string.h>
#include <math.h>

#include "raylib.h"
#define AHB_LIB_IMPLEMENTATION
#define AHB_STRIP_PREFIX
#include "ahb_lib.h"
#include "parse.h"
#include "render.h"


char *filename;
size_t render_list_index = 0;
RenderObject render_list[MAX_RENDER_LIST_SIZE] = {0};

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
    
    parse_to_render_object_list(content);

    // Log(INFO, "Number of render objects: %zu", render_list_index);
    bool draw_debug = false;
    while (!WindowShouldClose()) {
        BeginDrawing();

        if (IsKeyPressed(KEY_F5)) {
            // TASK(20260911-094144): reset state of application when F5 is pressed
            content = read_entire_file(filename);
            parse_to_render_object_list(content);
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
