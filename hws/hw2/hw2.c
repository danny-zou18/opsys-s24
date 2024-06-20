#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int solveQueens(int **board, int row, int m, int n, int *pipefd) {

}

int main(int argc, char** argv) {
    
    if (argc != 3) {
        fprintf(stderr, "ERROR: Wrong Number of Argument(s)\nUsage: %s <m> <n>\n",*(argv + 0));
        return EXIT_FAILURE;
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);

    if (m <= 0 || n <= 0) {
        fprintf(stderr, "ERROR: Invalid argument(s)\nUsage: %s <m> <n>\n", *(argv + 0));
        return EXIT_FAILURE;
    }
    if (n < m) {
        int temp = m;
        m = n;
        n = temp;
    }

    int **board = calloc(m, sizeof(int*));
    for (int i = 0; i < m; i++) {
        *(board + i) = calloc(n, sizeof(int));
    }

    int *pipefd = calloc(2, sizeof(int));
    int ipc = pipe(pipefd);
    if (ipc == -1) {
        fprintf(stderr, "pipe() failed");
        return EXIT_FAILURE;
    }

    printf("P%d: solving the Abridged (m,n)-Queens problem for %dx%d board\n", getpid(), m, n);

    

    return 0;
}

for this m x n queen problem solution that I have, how can I modify it so that at the top level process i print out the number of 