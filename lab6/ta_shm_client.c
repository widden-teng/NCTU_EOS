/*
 * shm_client.c -- attaches itself to the created shared memory
 * and uses the string (printf).
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHMSZ 27

int main(int argc, char *argv[]) {
    int shmid;
    key_t key;
    char *shm, *s;

    /* We need to get the segment named "5678", created by the server */
    key = 5678;

    /* Locate the segment */
    if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /* Now we attach the segment to our data space */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    printf("Client attaches the shared memory created by the server.\n");

    /* Now read what the server put in the memory */
    printf("Client reads characters from shared memory ...\n");

    for (s = shm; *s != '\0'; s++)
        putchar(*s);
    
    putchar('\n');

    /*
     * Finally, change the first character of the segment to '*',
     * indicating we have read the segment.
     */
    printf("Client writes '*' to the shared memory.\n");
    *shm = '*';

    /* Detach the shared memory segment */
    printf("Client detaches the shared memory.\n");
    shmdt(shm);

    return 0;
}
