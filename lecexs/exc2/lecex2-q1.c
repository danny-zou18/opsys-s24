#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int lecex2_child(unsigned int n) {
    FILE *file = fopen("lecex2.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        abort();
    }

    fseek(file, n - 1, SEEK_SET);
    int character = fgetc(file);
    if (character == EOF) {
        fprintf(stderr, "Error reading character from file\n");
        abort();
    }
    fclose(file);

    exit((int)(unsigned char)character);
}

int lecex2_parent() {
    int status;
    waitpid(-1, &status, 0);

    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("PARENT: child process returned '%c'\n", (char)exit_status);
        return EXIT_SUCCESS;
    } else {
        printf("PARENT: child process terminated abnormally!\n");
        return EXIT_FAILURE;
    }
}
