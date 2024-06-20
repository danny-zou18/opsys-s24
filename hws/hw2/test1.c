#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

// Function to deallocate memory and clear the board
void dealloc_board(char **board, int m, int *comm_pipe) {
    for (int i = 0; i < m; i++)
        free(*(board + i));
    free(board);
    free(comm_pipe);
}

// Add function to add a queen into the board while also checking other spots to make sure the placement is safe.
void Add(char **board, int m, int n, int row, int col) {
    // Place the queen
    *(*(board + row) + col) = 'Q';

    // Invalidate row
    for (int i = 0; i < n; i++) {
        if (i != col) {
            *(*(board + row) + i) = 'X';
        }
    }

    // Invalidate column
    for (int i = 0; i < m; i++) {
        if (i != row) {
            *(*(board + i) + col) = 'X';
        }
    }

    // Invalidate diagonals
    for (int i = 1; i < m; i++) {
        if (row + i < m && col + i < n) {
            *(*(board + row + i) + col + i) = 'X';
        }
        if (row + i < m && col - i >= 0) {
            *(*(board + row + i) + col - i) = 'X';
        }
        if (row - i >= 0 && col + i < n) {
            *(*(board + row - i) + col + i) = 'X';
        }
        if (row - i >= 0 && col - i >= 0) {
            *(*(board + row - i) + col - i) = 'X';
        }
    }
}

// Recursive function to solve the board
int solve_Queens(char **board, int m, int n, int *comm_pipe, int row) {
    if (row == m) {
        int result = m;
        write(*(comm_pipe + 1), &result, sizeof(int));
        #ifndef QUIET
            printf("P%d: found a solution; notifying top-level parent\n", getpid());
        #endif
        exit(EXIT_SUCCESS);
    }

    int validMoves = 0;
    for (int i = 0; i < n; i++) {
        if (*(*(board + row) + i) == '-')
            validMoves++;
    }

    if (validMoves == 0) {
#ifndef QUIET
        printf("P%d: dead end at row #%d; notifying top-level parent\n", getpid(), row);
#endif
        int result = row;
        write(*(comm_pipe + 1), &result, sizeof(int));
        exit(EXIT_SUCCESS);
    } else {
        if (row == 0) {
            printf("P%d: %d possible move%s at row #%d; creating %d child process%s...\n",
                   getpid(), validMoves, validMoves == 1 ? "" : "s", row, validMoves, validMoves == 1 ? "" : "es");
        } else {
#ifndef QUIET
            printf("P%d: %d possible move%s at row #%d; creating %d child process%s...\n",
                   getpid(), validMoves, validMoves == 1 ? "" : "s", row, validMoves, validMoves == 1 ? "" : "es");
#endif
        }
    }

    for (int i = 0; i < n; i++) {
        if (*(*(board + row) + i) != '-')
            continue;

        pid_t p = fork();

        if (p == -1) {
            fprintf(stderr, "ERROR: fork failed\n");
            dealloc_board(board, m, comm_pipe);
            abort();
        } else if (p == 0) {
            Add(board, m, n, row, i);
            solve_Queens(board, m, n, comm_pipe, row + 1);
            dealloc_board(board, m, comm_pipe);
            exit(EXIT_SUCCESS);
        } else {
#ifdef NO_PARALLEL
            int status;
            pid_t p_pid = waitpid(p, &status, 0);
            if (p_pid == -1) {
                fprintf(stderr, "P%d: waitpid failed!\n", getpid());
                abort();
            }

            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                fprintf(stderr, "P%d: child process failed!\n", getpid());
                abort();
            }
#endif
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc != 3 || argc <= 0) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: ./hw2.out <m> <n>\n");
        return EXIT_FAILURE;
    }

    int m = atoi(*(argv + 1));
    int n = atoi(*(argv + 2));

    if(m > n){
        int temp = m;
        m = n;
        n = temp;
    }

    if (m <= 0 || n <= 0) {
        fprintf(stderr, "ERROR: m and n must be greater than 0\n");
        return EXIT_FAILURE;
    }

    char **board = calloc(m, sizeof(char *));
    for (int i = 0; i < m; i++)
        *(board + i) = calloc(n, sizeof(char));

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            *(*(board + i) + j) = '-';
        }
    }

     int *comm_pipe = calloc(2, sizeof(int));
    if (pipe(comm_pipe) == -1) {
        fprintf(stderr, "pipe() failed");
        return EXIT_FAILURE;
    }

    printf("P%d: solving the Abridged (m,n)-Queens problem for %dx%d board\n", getpid(), m, n);

    pid_t p = fork();
    if (p == 0) { // Child process
        close(*(comm_pipe + 0)); // Close read end
        solve_Queens(board, m, n, comm_pipe, 0);
    } else if (p > 0) { // Parent process
        close(*(comm_pipe + 1)); // Close write end
        int result;
        int *endStates = calloc(m, sizeof(int));

        while (read(*(comm_pipe + 0), &result, sizeof(int)) > 0) {
            if (result > 0 && result <= m) {
                *(endStates + result - 1) += 1;
            }
        }

        printf("P%d: search complete\n", getpid());
        for (int i = 0; i < m; i++) {
            printf("P%d: number of %d-Queen end-states: %d\n", getpid(), i + 1, *(endStates + i));
        }

        waitpid(p, NULL, 0); // Wait for child process
        free(endStates);
    } else {
        perror("fork");
        return EXIT_FAILURE;
    }

    close(*(comm_pipe + 0));
    close(*(comm_pipe + 1));
    dealloc_board(board, m, comm_pipe);

    return EXIT_SUCCESS;
}