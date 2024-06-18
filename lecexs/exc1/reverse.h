#ifndef REVERSE_H
#define REVERSE_H

#include <stdlib.h>
#include <string.h>

char* reverse(char* s) {
    int len = strlen(s);

    char* buffer = (char*)malloc(len + 1);

    if (buffer == NULL) {
        return NULL;
    }

    for (int i = 0; i < len; i++) {
        *(buffer + i) = *(s + len - i - 1);
    }

    *(buffer + len) = '\0';

    for (int i = 0; i <= len; i++) {
        *(s + i) = *(buffer + i);
    }

    free(buffer);

    return s;
}

#endif 
