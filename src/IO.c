#include "../include/IO.h"

editor_t editorOp = {
    .cx = 6,
    .cy = 0,
    .insert = false,
    .scroll = false,
    .line = -2,
    .llidx = 1,
};


static void _move_cursor(int x, int y) {
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y, x);
    write(STDOUT_FILENO, buf, strlen(buf));
}


char getkey() {
    char c;
    while (read(STDIN_FILENO, &c, 1) != 1);
    return c;
}


char processKey(char c) {
    switch (c) { 
        case CTRL_KEY('q'):
            free(editorOp.lastLines);
            free(editorOp.buffer.contents);
            refreshScr(false);
            exit(0); 
        case CTRL_KEY('i'):
            write(STDOUT_FILENO, "\033[6 q", 6);
            editorOp.insert = true;
            break;
        case CTRL_KEY('s'):
            if (!(editorOp.insert) && editorOp.buffer.size > 0) {
                int row, col;
                getWinSize(&row, &col);
                _move_cursor(15, col);

                FILE* fp = fopen(editorOp.filename, "w");
                fprintf(fp, "%s\n", editorOp.buffer.contents);
                fclose(fp);

                char* writtenMsg = "\033[38;5;255mBuffer written to disk.";
                char* blank = "                                     ";
                write(STDOUT_FILENO, writtenMsg, strlen(writtenMsg));
                getkey();
                _move_cursor(15, col);
                write(STDOUT_FILENO, blank, strlen(blank));
                _move_cursor(editorOp.cx, editorOp.cy);
            } else {
                int row, col;
                getWinSize(&row, &col);
                _move_cursor(15, col);
                char* writtenMsg = "\033[38;5;255mCannot write empty buffer!";
                char* blank = "                                     ";
                write(STDOUT_FILENO, writtenMsg, strlen(writtenMsg));
                getkey();
                _move_cursor(15, col);
                write(STDOUT_FILENO, blank, strlen(blank));
                _move_cursor(editorOp.cx, editorOp.cy);
            }
    }

    if (editorOp.insert) {
        while (read(STDOUT_FILENO, &c, 1) == 1 && c != 27) {
            switch (c) {
                case '\n':
                    int oldCy = editorOp.cy;
                    ++editorOp.cy;
                    editorOp.lastLines = (unsigned long*)realloc(editorOp.lastLines, sizeof(unsigned long) * (editorOp.llidx + 1));
                    editorOp.lastLines[editorOp.llidx - 1] = editorOp.cx;
                    ++editorOp.llidx;
                    buffer_push('\n', &editorOp.buffer);
                    while (editorOp.cy == oldCy) {
                        ++editorOp.cy;
                    }

                    editorOp.cx = 7;

                    _move_cursor(editorOp.cx, editorOp.cy);

                    if (editorOp.cy >= 20) {
                        write(STDOUT_FILENO, "\r\n", 2);
                        ++editorOp.line;

                        char rowBuf[20];
                        snprintf(rowBuf, sizeof(rowBuf), "%d~", editorOp.line);
                        write(STDOUT_FILENO, rowBuf, strlen(rowBuf));
                        _move_cursor(editorOp.cx, editorOp.cy);
                    }

                    continue;
                case 127:
                    if (editorOp.cx > 6) {
                        --editorOp.cx;
                        _move_cursor(editorOp.cx, editorOp.cy);
                        write(STDOUT_FILENO, " ", 1);
                        _move_cursor(editorOp.cx, editorOp.cy);
                    } else if (editorOp.cy > 2) {
                        --editorOp.cy;
                        editorOp.cx = editorOp.lastLines[editorOp.llidx - 2];
                        editorOp.lastLines = (unsigned long*)realloc(editorOp.lastLines, sizeof(unsigned long) * (editorOp.llidx - 1));
                        --editorOp.llidx;
                        _move_cursor(editorOp.cx, editorOp.cy);
                    }
                    
                    if (editorOp.buffer.size > 0) {
                        buffer_pop(&editorOp.buffer);
                    }

                    continue;
            }

            write(STDOUT_FILENO, &c, 1);
            ++editorOp.cx;
            buffer_push(c, &editorOp.buffer);
        }

        write(STDOUT_FILENO, "\033[1 q", 6);
        editorOp.insert = false;
        return 'I';
    } else {
        return 'N';
    }
}


void refreshScr(bool drawRows) {
    static bool start = true;

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editorOp.cy + 1, editorOp.cx + 1);
    write(STDOUT_FILENO, "\x1b[2J", 4); 
    if (drawRows) {
        for (int i = 0; i < 24; ++i, ++editorOp.line) {
            char rowbuf[20];
            snprintf(rowbuf, sizeof(rowbuf), "\033[38;5;255m%d~\r\n", editorOp.line);
            write(STDOUT_FILENO, rowbuf, strlen(rowbuf));
        }

        write(STDOUT_FILENO, "\x1b[H", 3);
    }
    
    write(STDOUT_FILENO, "\x1b[H", 3);

    if (drawRows) {
        write(STDOUT_FILENO, buf, strlen(buf));
    }

    if (start) {
        start = false;
        ++editorOp.cy;
        _move_cursor(editorOp.cx, editorOp.cy);
    } 
}




int getWinSize(int* rows, int* cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}



void getCursPos(int* rows, int* cols) {
    char buf[32];
    unsigned int i = 0;


    if (read(STDIN_FILENO, "\x1b[6n", 4) != 4) {
            *rows = -1;
            *cols = -1;
            return;
    }

    while (i < sizeof(buf) - 1) { 
        if (read(STDIN_FILENO, &buf[i], 1) != 1) {
            break;
        } else if (read(STDIN_FILENO, &buf[i], 1) != 1) {
            break;
        }

        ++i;
    }

    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') {
        *rows = -1;
        *cols = -1;
        return;
    }
}
