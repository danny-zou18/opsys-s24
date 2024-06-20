#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>


void printSolution(int **board, int m, int n) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%c ", board[i][j] == 1 ? 'Q' : '.');
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

void addQueen(int **board, int m, int n, int row, int col) {
    // Invalidate L and R
    for (int i = 0; i < n; i++)
        board[row][i] = -1;

    // Invalidate U and D
    for (int i = 0; i < m; i++)
        board[i][col] = -1;

    // Invalidate Diagonals and Antidiagonals
    for (int i = 1; i < n; i++) {
        // Invalidate diagonals
        if (row - i >= 0 && col - i >= 0)
            board[row - i][col - i] = -1;
        if (row - i >= 0 && col + i < n)
            board[row - i][col + i] = -1;
        // Invalidate antidiagonals
        if (row + i < m && col - i >= 0)
            board[row + i][col - i] = -1;
        if (row + i < m && col + i < n)
            board[row + i][col + i] = -1;
    }

    // Place Queen
    board[row][col] = 1;
    return;
}

int solveQueens(int **board, int row, int m, int n, int *pipefd) {
    // Base case: Solution Found
    if (row == m) {
        printf("found a solution; notifying top-level parent\n");
        int queensPlaced = 0;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                if (board[i][j] == 1) {
                    queensPlaced++;
                }
            }
        }
        write(pipefd[1], &queensPlaced, sizeof(int));
        return EXIT_SUCCESS;
    }

    // Counts Valid Moves
    int validMoves = 0;
    for (int i = 0; i < n; i++) {
        if (board[row][i] == 0)
            validMoves++;
    }

    // If no valid moves left: Dead end no solution
    if (validMoves == 0) {
        #ifndef QUIET
        printf("P%d: dead end at row #%d; notifying top-level parent\n",
               getpid(), row);
        int queensPlaced = 0;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                if (board[i][j] == 1) {
                    queensPlaced++;
                }
            }
        }
        write(pipefd[1], &queensPlaced, sizeof(int));
        return EXIT_SUCCESS;
        #endif
    }
    // Else print possible amount of moves
    else {
        printf("P%d: %d possible move%s at row #%d; creating %d child "
               "process%s...\n",
               getpid(), validMoves, validMoves == 1 ? "" : "s", row,
               validMoves, validMoves == 1 ? "" : "es");
    }

    // Add Queens wherever possible
    for (int i = 0; i < n; i++) {
        // If not empty skip this location on the board.
        if (board[row][i] != 0)
            continue;

        pid_t p = fork();

        // If fork failed abort
        if (p == -1) {
            // fork failed
            fprintf(stderr, "Error: fork failed\n");
            free_board(board, m, pipefd);
            abort();
        }
        // Is child        
        else if (p == 0) {
            addQueen(board, m, n, row, i);
            solveQueens(board, row + 1, m, n, pipefd);

            free_board(board, m, pipefd);
            exit(EXIT_SUCCESS);
        }
        // Is parent
        else {
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
        }
    }
    return EXIT_SUCCESS;
}



int main(int argc, char **argv) {
    // Validate command-line arguments
    if (argc != 3) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: %s <m> <n>\n", *(argv + 0));
        return EXIT_FAILURE;
    }

    int m = atoi(*(argv + 1));
    int n = atoi(*(argv + 2));

    // Validate m and n
    if (m <= 0 || n <= 0) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: %s <m> <n>\n", *(argv + 0));
        return EXIT_FAILURE;
    }

    // Swap m and n if necessary
    if (n < m) {
        int temp = m;
        m = n;
        n = temp;
    }

    // Create the chess board
    int **board = calloc(m, sizeof(int *));
    for (int i = 0; i < m; i++) {
        *(board + i) = calloc(n, sizeof(int));
    }

    // pipe creation
    int *pipefd = calloc(2, sizeof(int));
    int ipc = pipe(pipefd);
    if (ipc == -1) {
        fprintf(stderr, "pipe() failed");
        return EXIT_FAILURE;
    }

    printf("P%d: solving the Abridged (m,n)-Queens problem for %dx%d board\n",
           getpid(), m, n);
    solveQueens(board, 0, m, n, pipefd);
    printf("P%d: search complete\n", getpid());

    close(pipefd[1]);

    // Read the pipe
    int queensPlaced;
    int *queensEndStates = calloc(m, sizeof(int));
    for (int i = 0; i < m; i++) {
        *(queensEndStates+i) = 0;
    }
    while (read(pipefd[0], &queensPlaced, sizeof(int)) > 0) {
        (*(queensEndStates + (queensPlaced - 1)))++;
    }

    for (int i = 1; i <= m; i++) {
        printf("P%d: number of %d-Queen end-states: %d\n", getpid(), i, *(queensEndStates+(i - 1)));
    }

    free(queensEndStates);

    // free heap
    close(ipc);
    close(ipc + 1);
    free_board(board, m, pipefd);


    return EXIT_SUCCESS;

}