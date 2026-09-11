build: src/main.c
	clang -g \
        -o mdviewer \
        -Wall -Wextra \
        -I./vendor/raylib-6.0_linux_amd64/include \
        ./src/render.c \
        ./src/parse.c \
        ./src/main.c \
        -L./vendor/raylib-6.0_linux_amd64/lib \
        -l:libraylib.a \
        -lm -lX11

