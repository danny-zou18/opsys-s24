#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void print_chunks(int n, const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char *buffer = (char *)malloc(n);
    if (buffer == NULL) {
        perror("Error allocating memory");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int bytesRead;
    int firstchunk = 1; 

   while ((bytesRead = read(fd, buffer, n)) > 0) {
        if (bytesRead == n) {
            if (!firstchunk) {
                write(STDOUT_FILENO, "|", 1);
            }
            write(STDOUT_FILENO, buffer, n);
            firstchunk = 0;
            lseek(fd, n, SEEK_CUR); 
        } else {
            break; 
        }
    }

    free(buffer);
    close(fd);
    putchar('\n'); 
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <chunk_size> <filename>\n", *argv);
        exit(EXIT_FAILURE);
    }

    int n = atoi(*(argv + 1));
    if (n <= 0) {
        fprintf(stderr, "Chunk size must be a positive integer\n");
        exit(EXIT_FAILURE);
    }

    const char *filename = *(argv + 2);
    print_chunks(n, filename);

    return 0;
}
