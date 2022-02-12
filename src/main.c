#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "../include/IO.h"


static struct termios orig_termios;

static void _disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void _enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(_disableRawMode);
    tcgetattr(STDIN_FILENO, &raw);
    /*
     * 1. Clear echo bit.
     * 2. Clear ICANON bit.
     * 3. Clear ISIG bit.
     */
    
    raw.c_iflag &= ~(IXON);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main(int argc, char* argv[]) {
    bool new = false;

    if (argc < 2) {
        printf("Usage: KessEdit <filename>\n");
        exit(0);
    }

    if (strlen(argv[1]) > 99) {
        printf("Why the hell do you need a filename that long?\n");
        exit(1);
    }

    if (access(argv[1], F_OK) != 0) {
        new = true;
    } else {
        printf("Only new files are supported for now, support for existing files will be added soon.\n");
        exit(1);
    }

    extern editor_t editorOp;

    editorOp.lastLines = (unsigned long*)malloc(sizeof(unsigned long*));
    editorOp.isnew = new;
    strcpy(editorOp.filename, argv[1]);

    buffer_t buffer = {
        .size = 0,
        .contents = (char*)calloc(2, sizeof(char)),
    };

    editorOp.buffer = buffer;

    _enableRawMode();
    char c;

    int rows;
    int cols;

    int cur_cols;
    int cur_rows;

    bool started = false;

    getWinSize(&rows, &cols);
    refreshScr(true);

    char* msg = "\033[38;5;255m ";
    write(STDIN_FILENO, msg, strlen(msg));

    while (1) { 
        processKey(getkey());
    }
}
