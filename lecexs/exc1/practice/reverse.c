#ifndef REVERSE_H
#define REVERSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * reverse(char* s) {
    if (s == NULL) {
        return NULL;
    }

    int str_len = strlen(s);

    char* buffer = (char*)malloc(str_len + 1);

    if (buffer == NULL) {
        return NULL;
    }

    for (int i = 0; i < str_len; i++) {
        *(buffer + i) = *(s + (str_len-i-1));
    }
    for (int i = 0; i <= str_len; i++) {
        *(s + i) = *(buffer + i);
    }

    *(buffer + str_len) = '\0';

    free(buffer);

    return s;
}

int main() {

    char s[] = "Hello, World!";
    printf("%s\n",reverse(s));

    return EXIT_SUCCESS;
}

#endif