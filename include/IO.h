#ifndef IO_H
#define IO_H

#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <string.h>
#include "Buffer.h"

#define CTRL_KEY(k) ((k) & 0x1F)

typedef struct {
    int cx;
    int cy;
    int sh;
    int sw;
    bool insert;
    bool scroll;
    long line;
    unsigned long* lastLines;   // Last X position.
    unsigned long llidx;
    buffer_t buffer;
    char filename[100];
    bool isnew;
} editor_t;


char getkey();
char processKey(char c);
void refreshScr(bool drawRows);
int getWinSize(int* rows, int* cols);
void getCursPos(int* rows, int* cols);

#endif
