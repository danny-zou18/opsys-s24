#include <stdio.h>
#include "reverse.h"

int main() {
    char str[] = "hello";
    printf("Original: %s\n", str);
    reverse(str);
    printf("Reversed: %s\n", str);
    return 0;
}