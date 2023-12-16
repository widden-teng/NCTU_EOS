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
volatile sig_atomic_t flag = 1;
int number;

void safe_close_routine(int sig) {
  flag = 0;
}



// 信号处理函数
void sig_handler(int signo) {

  if (signo == SIGUSR1) {
    int guess = shared_mem->guess;
    // 比较猜测值与目标数字
    if (guess < number) {
      strcpy(shared_mem->result, "bigger");
    } else if (guess > number) {
      strcpy(shared_mem->result, "smaller");  
    } else {
      strcpy(shared_mem->result, "bingo"); 
    }
    printf("Guess %d,  %s\n", guess,  shared_mem->result);
  }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <key> <guess>\n", argv[0]);
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
    // init share memory
    shared_mem->guess = 0;
    strcpy(shared_mem->result, "unstart");



    number = atoi(argv[2]);

    /* register handler to SIGUSR1 */
    // memset(&sa, 0, sizeof(sa));）這表示沒定義的其他參數使用默認的參數值。
    // 使用sigaction函數將handler函數註冊到SIGUSR1信號，這樣每當收到SIGUSR1信號時，將調用handler函數，並在其中做判斷。
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    sigaction(SIGUSR1, &sa, NULL);

    // for ctrl+c signal
    signal(SIGINT, safe_close_routine);


    printf("Game PID: %d\n", getpid());


    while(flag) {
        sleep(1);
    }
    

    /* Detach the shared memory segment*/
    int retval;
    shmdt(shared_mem);
    /* Remove share memory*/ 
    retval = shmctl(shmid, IPC_RMID, NULL);
    if (retval < 0) {
        fprintf(stderr, "Server removes delivery shared memory failed\n");
        exit(1);
    }
    /*------------------------------*/

    return 0;

}
