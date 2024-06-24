#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // Required for open() and O_* constants
#include <unistd.h> // Required for close()


int main(int argc, char** argv) {
    if (argc != 3) {
        perror("Wrong number of arguments, exiting...\n");
        exit(EXIT_FAILURE);
    }

    int n = atoi(*(argv+1));
    char* str = *(argv + 2);

    int fd = open(str, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file, exiting...\n");
        exit(EXIT_FAILURE);
    }

    char* buffer = (char*)malloc(n);
    if (buffer == NULL) {
        perror("Error allocating memory for buffer, exitting...\n");
        exit(EXIT_FAILURE);
    }

    int bytesRead;
    int firstChunk = 1;

    while((bytesRead = read(fd, buffer, n)) > 0) {
        if (bytesRead == n) {
            if (!firstChunk) {
                write(STDOUT_FILENO, "|", 1);
            }
            write(STDOUT_FILENO, buffer, n);
            firstChunk = 0;
            lseek(fd, n, SEEK_CUR);
        } else {
            if (!firstChunk) {
                write(STDOUT_FILENO, "|", 1);
            }
            write(STDOUT_FILENO, buffer, bytesRead);
            firstChunk = 0;
        }
    }

    free(buffer);
    close(fd);
    putchar('\n');

    return EXIT_SUCCESS;
}