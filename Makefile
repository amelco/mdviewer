build: src/main.c
	gcc -g \
        -o mdviewer \
        -Wall -Wextra \
        -I./vendor/raylib-6.0_linux_amd64/include \
        ./src/main.c \
        -L./vendor/raylib-6.0_linux_amd64/lib \
        -l:libraylib.a \
        -lm -lX11

