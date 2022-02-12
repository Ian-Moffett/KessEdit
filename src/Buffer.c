#include "../include/Buffer.h"



void buffer_push(char c, buffer_t* buffer) {
    buffer->contents[buffer->size] = c;
    ++buffer->size;
    buffer->contents = (char*)realloc(buffer->contents, sizeof(char) * (buffer->size + 2));
    buffer->contents[buffer->size] = '\0';
}



char buffer_pop(buffer_t* buffer) {
    char c = buffer->contents[buffer->size - 1];
    buffer->contents = (char*)realloc(buffer->contents, sizeof(char) * (buffer->size - 2));
    --buffer->size;
    return c;
}
