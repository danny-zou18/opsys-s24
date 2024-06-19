#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
    
    if (argc != 3) {
        fprintf(stderr, "ERROR: Wrong Number of Argument(s)\nUsage: hw2.out <m> <n>\n");
        return EXIT_FAILURE;
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);

    if (m <= 0 || n <= 0) {
        fprintf(stderr, "ERROR: Invalid argument(s)\nUsage: hw2.out <m> <n>\n");
        return EXIT_FAILURE;
    }

    

    return 0;
}