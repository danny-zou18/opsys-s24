#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    pid_t pid, gpid;
    int status;
    int count = 0;

    // Create child process
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "CHILD: fork() failed\n");
        return -1;
    } else if (pid == 0) {
        // Child process

        // Create grandchild process
        gpid = fork();
        if (gpid < 0) {
            fprintf(stderr, "GRANDCHILD: fork() failed\n");
            return -1;
        } else if (gpid == 0) {
            // Grandchild process

            // Open input file
            int fd = open(*(argv+1), O_RDONLY);
            if (fd < 0) {
                fprintf(stderr, "GRANDCHILD: open() failed: No such file or directory\n");
                abort();
            }
            // Count properly nested parentheses
            char c;
            int nested = 0;
            while (read(fd, &c, 1) > 0) {
                if (c == '(') {
                    nested++;
                } else if (c == ')') {
                    if (nested > 0) {
                        count++;
                        nested--;
                    }
                }
            }
            // Close input file
            close(fd);
            if (count == 1) {
                printf("GRANDCHILD: counted %d properly nested pair of parentheses\n", count);
            } else if ( count >= 2 || count == 0) {
                printf("GRANDCHILD: counted %d properly nested pairs of parentheses\n", count);
            }
            // Return count as exit status
            if (count > 255) {
                fprintf(stderr, "GRANDCHILD: value bigger than a byte\n");
                abort();
            }
            exit(count);
        } else {
            // Child process

            // Wait for grandchild process to finish
            waitpid(gpid, &status, 0);
            if (WIFEXITED(status)) {
                // Get grandchild's exit status
                count = WEXITSTATUS(status);

                // Double the count
                int doubled_count = 2 * count;

                if (doubled_count > 255) {
                    fprintf(stderr, "CHILD: value bigger than a byte\n");
                    abort();
                }
                
                printf("CHILD: doubled %d; returning %d\n", count, doubled_count);
                // Return doubled count as exit status
                exit(doubled_count);
            } else {
                fprintf(stderr, "CHILD: rcvd -1 (error)\n");
                abort();
            }
        }
    } else {
        // Parent process

        // Wait for child process to finish
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            // Get child's exit status
            count = WEXITSTATUS(status);
            // Output the count
            printf("PARENT: there are %d properly nested parentheses\n", count);
        } else {
            fprintf(stderr, "PARENT: rcvd -1 (error)\n");
            return EXIT_FAILURE;
        }
    }

    return 0;
}