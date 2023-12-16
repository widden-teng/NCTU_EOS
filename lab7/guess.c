#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
    int guess;
    char result[8];
}data;
#define SHMSZ sizeof(data)

// global variable
int shmid; 
data *shared_mem;
int upper_bound;
int min = 0;
volatile sig_atomic_t flag = 1;
int pid;

void set_guess(){

    
    if(strcmp(shared_mem->result, "bingo") == 0){
        flag = 0;
    }else{    
        if (strcmp(shared_mem->result, "unstart") == 0){
            min = 0;
        }else if(strcmp(shared_mem->result, "bigger") == 0){
            min = shared_mem->guess;
        }else if(strcmp(shared_mem->result, "smaller") == 0){
            upper_bound = shared_mem->guess;
        }
        int guess = (min + upper_bound) / 2;
        shared_mem->guess = guess;   

        kill(pid, SIGUSR1);
    }
    
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <key> <upper_bound> <pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // get key by ftok
    key_t key = ftok(argv[1], 0);
    /*share memory setting */ 
    /* Create the segment */
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    /* Now we attach the segment to our data space */
    if ((shared_mem = shmat(shmid, NULL, 0)) == (data *) -1) {
        perror("shmat");
        exit(1);
    }

    upper_bound = atoi(argv[2]);
    pid = atoi(argv[3]);

    
    while (flag) {
        sleep(1);  
        set_guess();
        printf("Guess: %d\n", shared_mem->guess);
    }
    /* Detach the shared memory segment*/
    shmdt(shared_mem);
    return 0;

}