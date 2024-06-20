#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

//For testing
void printSolution(int **board, int m, int n) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%c ", *(*(board + i)+j) == 1 ? 'Q' : '.');
        }
        printf("\n");
    }
    printf("\n");
}

void free_board(int **board, int m, int *pipefd) {
    for (int i = 0; i < m; i++) {
        free(*(board + i));
    }
    free(board);
    free(pipefd);
}

void add_queen(int **board, int m, int n, int row, int col) {
    for (int i = 0; i < n; i++) {
        *(*(board + row) + i) = -1;
    }
    for (int i = 0; i < m; i++) {
        *(*(board + i) + col) = -1;
    }
    for (int i = 1; i < n; i++) {
        if (row - i >= 0 && col - i >= 0) {
            *(*(board + row - i) + col - i) = -1;
        }
        if (row - i >= 0 && col + i < n) {
            *(*(board + row - i) + col + i) = -1;
        }
        if (row + i < m && col - i >= 0) {
            *(*(board + row + i) + col - i) = -1;
        }
        if (row + i < m && col + i < n) {
            *(*(board + row + i) + col + i) = -1;
        }
    }
    *(*(board + row) + col) = 1;
    return;
}

int solve_queens(int **board, int row, int m, int n, int *pipefd) {
    if (row == m) {
        int queensPlaced = m;
        write(*(pipefd+1), &queensPlaced, sizeof(int));
        // printSolution(board, m, n);
        #ifndef QUIET
            printf("P%d: found a solution; notifying top-level parent\n", getpid());
        #endif
        free_board(board, m, pipefd);
        exit(EXIT_SUCCESS);
    }

    int validMoves = 0;
    for (int i = 0; i < n; i++) {
        if (*(*(board + row) + i) == 0) {
            validMoves++;
        }
    }
    if (validMoves == 0) {
        int queensPlaced = row;
        write(*(pipefd+1), &queensPlaced, sizeof(int));
        #ifndef QUIET
        printf("P%d: dead end at row #%d; notifying top-level parent\n",
               getpid(), row);
        #endif
        free_board(board, m, pipefd);
        exit(EXIT_SUCCESS);
        
    } else {
        if (row == 0) {
             printf("P%d: %d possible move%s at row #%d; creating %d child "
               "process%s...\n",
               getpid(), validMoves, validMoves == 1 ? "" : "s", row,
               validMoves, validMoves == 1 ? "" : "es");
        } else {
             #ifndef QUIET
            printf("P%d: %d possible move%s at row #%d; creating %d child "
               "process%s...\n",
               getpid(), validMoves, validMoves == 1 ? "" : "s", row,
               validMoves, validMoves == 1 ? "" : "es");
            #endif
        }
    }

    for (int i = 0; i < n; i++) {
        if (*(*(board + row) + i) != 0) {
            continue;
        }

        pid_t p = fork();

        if (p == -1) {
            fprintf(stderr, "Error: fork failed\n");
            free_board(board, m, pipefd);
            abort();    
        } else if (p == 0) {
            add_queen(board, m, n, row, i);
            solve_queens(board, row + 1, m, n, pipefd);

            free_board(board, m, pipefd);
            exit(EXIT_SUCCESS);
        } else {
            #ifdef NO_PARALLEL
                int status;
                pid_t p_pid = waitpid(p, &status, 0);
                if (p_pid == -1) {
                    fprintf(stderr, "P%d: waitpid failed!\n", getpid());
                    abort();
                }

                if (WIFEXITED(status)) {
                    continue;
                }   
                else {
                    fprintf(stderr, "P%d: child process did not exit normally\n",
                            getpid());
                    abort();
                }
            #endif
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    // Validate command-line arguments
    if (argc != 3) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: ./hw2.out <m> <n>\n");
        return EXIT_FAILURE;
    }
    int m = atoi(*(argv + 1));
    int n = atoi(*(argv + 2));
    // Validate m and n
    if (m <= 0 || n <= 0) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: ./hw2.out <m> <n>\n");
        return EXIT_FAILURE;
    }
    // Swap m and n if necessary
    if (n < m) {
        int temp = m;
        m = n;
        n = temp;
    }

    printf("P%d: solving the Abridged (m,n)-Queens problem for %dx%d board\n",
           getpid(), m, n);

    // Create the board
    int **board = calloc(m, sizeof(int *));
    for (int i = 0; i < m; i++) {
        *(board + i) = calloc(n, sizeof(int));
    }

    // Create the pipe
    int *pipefd = calloc(2, sizeof(int));
    int ipc = pipe(pipefd);
    if (ipc == -1) {
        fprintf(stderr, "pipe() failed");
        return EXIT_FAILURE;
    }

    pid_t p = fork(); // Initial Fork

    if (p == 0) {
        close(*pipefd);
        solve_queens(board, 0, m, n, pipefd);
    } else {
        waitpid(p, NULL, 0); // Wait for child to finish
        close(*(pipefd + 1));
        // Get results from the pipe
        int queensPlaced;
        int *queensEndStates = calloc(m, sizeof(int));
        for (int i = 0; i < m; i++) {
            *(queensEndStates+i) = 0;
        }
        while (read(*(pipefd + 0), &queensPlaced, sizeof(int)) > 0) {
            (*(queensEndStates + (queensPlaced - 1)))++;
        }
        printf("P%d: search complete\n", getpid());

        for (int i = 1; i <= m; i++) {
            printf("P%d: number of %d-Queen end-states: %d\n", getpid(), i, *(queensEndStates+(i - 1)));
        }
        free(queensEndStates);
    }
    
    // Free lingering memory
    close(ipc);
    close(ipc + 1);
    free_board(board, m, pipefd);
    return EXIT_SUCCESS;
}
