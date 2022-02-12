#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>

typedef struct {
    size_t size;
    char* contents;
} buffer_t;


void buffer_push(char c, buffer_t* buffer);
char buffer_pop(buffer_t* buffer);


#endif
