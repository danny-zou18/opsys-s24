/* lecex3-q1.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/shm.h>
#include <sys/ipc.h>

int lecex3_q1_child(int pipefd)
{
    int shm_key, shm_size;
    int rc = read(pipefd, &shm_key, sizeof(int));
    if (rc == -1) {
        perror("read() failed");
        return EXIT_FAILURE;
    }

    rc = read(pipefd, &shm_size, sizeof(int));
    if (rc == -1) {
        perror("read() failed");
        return EXIT_FAILURE;
    }

    int shmid = shmget(shm_key, shm_size, 0666);
    if (shmid == -1) {
        perror("shmget() failed");
        return EXIT_FAILURE;
    }

    char *shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("shmat() failed");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < shm_size; i++) {
        if (islower(*(shm_ptr+i))) {
            *(shm_ptr+i) = toupper(*(shm_ptr+i));
        }
    }

    if (shmdt(shm_ptr) == -1) {
        perror("shmdt() failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}