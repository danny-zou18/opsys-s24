#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

void * copy_file( void * arg );

int main(int argc, char **argv) {
    pthread_t *threads = malloc((argc - 1) * sizeof(pthread_t));
    int **bytes_copied = malloc((argc - 1) * sizeof(int*));

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            printf("MAIN: Creating thread to copy \"%s\"\n", *(argv+i));
        }
    }

    for (int i = 1; i < argc; i++) {
        pthread_create(&(*(threads+(i - 1))), NULL, copy_file, (void *) *(argv+i));
    }

    for (int i = 1; i < argc; i++) {
        
        pthread_join(*(threads+(i - 1)), (void **) (bytes_copied+(i - 1)));
        printf("MAIN: Thread completed copying %d bytes for \"%s\"\n", **(bytes_copied+(i - 1)), *(argv+i));
    }

    int total_bytes_copied = 0;
    for (int i = 0; i < argc - 1; i++) {
        total_bytes_copied += **(bytes_copied+(i));
    }
    if (argc - 1 > 1 || argc - 1 == 0) {
        printf("MAIN: Successfully copied %d bytes via %d child threads\n", total_bytes_copied, argc - 1);
    } else {
        printf("MAIN: Successfully copied %d bytes via %d child thread\n", total_bytes_copied, argc - 1);
    }

    free(threads);
    free(bytes_copied);

    return 0;
}