/*
 * timer.c
 *
 * 演示了如何使用虛擬定時器(ITIMER_VIRTUAL)在指定的時間間隔內執行一個定時處理程序。
 * 這裡使用了setitimer函數來啟動虛擬定時器，並且設定了一個處理程序，當定時器到期時，會執行該處理程序。
 */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#define SHM_KEY 5555


// (大小為26 ＝ 8+8+4+4+2) timer也要一起改！！！！！！！！！！！！
typedef struct {
    char *front_camera_path;
    char *back_camera_path;
    short front_person_info[2];  
    short back_person_info[2];  
    short timestamp;     
}MyStruct;
MyStruct *sharedData;

// 每當虛擬定時器到期，將執行 timer_handler 函數，打印出定時器已到期的次數。這樣會一直執行下去，直到手動終止程式
void timer_handler(int signum) {
    static int count = 0;
    if(count == 3){
        printf("shm time trigger  is :%d \n", ++(sharedData->timestamp));
        count = 0;
    }
    count++;
}

int main(int argc, char **argv) {

    // get share memory
    int shmid = shmget(SHM_KEY, sizeof(MyStruct), 0666);

    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    sharedData = (MyStruct *)shmat(shmid, NULL, 0);  // 連接共享內存
    if ((void *)sharedData == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }




    struct sigaction sa;
    struct itimerval timer;

    /* Install timer_handler as the signal handler for SIGVTALRM */
    // 將 timer_handler 函數設為 SIGVTALRM 信號的處理程序。然後，使用 sigaction 函數將該處理程序註冊到 SIGVTALRM 信號。
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after 250 msec */
    // 將 it_value 設定為 n 秒，表示第一次定時器到期的時間
    timer.it_value.tv_sec = 3;
    timer.it_value.tv_usec = 0;

    /* Reset the timer back to 250 msec after expired */
    // it_interval 被設定為相同的值，表示每次定時器到期後重複的時間間隔。
    timer.it_interval.tv_sec = 3;
    timer.it_interval.tv_usec = 0;

    /* Start a virtual timer */
    // 使用 setitimer 函數啟動虛擬定時器(ITIMER_VIRTUAL)，將之前設定的 timer 作為參數
    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    /* Do busy work */
    while (1);

    return 0;
}
