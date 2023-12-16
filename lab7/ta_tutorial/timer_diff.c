/*
 * timer_diff.c
 * 演示了如何使用三種不同的定時器（ITIMER_REAL、ITIMER_VIRTUAL、ITIMER_PROF）以及信號處理程序
 * 來計算某些操作的執行時間。具體來說，程式設置了三個不同的定時器，每個定時器都與一個信號相關聯，當定時器到期時，
 * 將執行相應的信號處理程序。程式還執行一個模擬I/O操作的函數，並計算在此操作期間定時器到期的次數。
 */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* counter */
int SIGALRM_count = 0;
int SIGVTALRM_count = 0;
int SIGPROF_count = 0;

/* handler of SIGALRM */
void SIGALRM_handler(int signum) {
    SIGALRM_count++;
}

/* handler of SIGVTALRM */
void SIGVTALRM_handler(int signum) {
    SIGVTALRM_count++;
}

/* handler of SIGPROF */
void SIGPROF_handler(int signum) {
    SIGPROF_count++;
}

void IO_WORKS();

int main(int argc, char **argv) {
    struct sigaction SA_SIGALRM, SA_SIGVTALRM, SA_SIGPROF;
    struct itimerval timer;

    /* Install SIGALRM_handler as the signal handler for SIGALRM */
    memset(&SA_SIGALRM, 0, sizeof(SA_SIGALRM));
    SA_SIGALRM.sa_handler = &SIGALRM_handler;
    sigaction(SIGALRM, &SA_SIGALRM, NULL);

    /* Install SIGVTALRM_handler as the signal handler for SIGVTALRM */
    memset(&SA_SIGVTALRM, 0, sizeof(SA_SIGVTALRM));
    SA_SIGVTALRM.sa_handler = &SIGVTALRM_handler;
    sigaction(SIGVTALRM, &SA_SIGVTALRM, NULL);

    /* Install SIGPROF_handler as the signal handler for SIGPROF */
    memset(&SA_SIGPROF, 0, sizeof(SA_SIGPROF));
    SA_SIGPROF.sa_handler = &SIGPROF_handler;
    sigaction(SIGPROF, &SA_SIGPROF, NULL);

    // 設定三個定時器 timer，使其在100毫秒後到期，然後將其重置，以便在到期後繼續觸發
    /* Configure the timer to expire after 100 msec */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;

    /* Reset the timer back to 100 msec after expired */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 100000;

    /* Start timer */
    // 啟動三個定時器，分別與 ITIMER_REAL、ITIMER_VIRTUAL、ITIMER_PROF 相關聯
    setitimer(ITIMER_REAL, &timer, NULL);
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    setitimer(ITIMER_PROF, &timer, NULL);

    /* Do some I/O operations */
    IO_WORKS();

    printf("SIGALRM_count = %d\n", SIGALRM_count);
    printf("SIGVTALRM_count = %d\n", SIGVTALRM_count);
    printf("SIGPROF_count = %d\n", SIGPROF_count);

    return 0;
}

// 執行模擬I/O操作的 IO_WORKS 函數，其中包含對文件的反覆打開、讀取和關閉操作。
void IO_WORKS() {
    int fd, ret;
    char buffer[100];
    int i;

    /* Open/Read/Close file 300000 times */
    for (i = 0; i < 300000; i++) {
        if ((fd = open("./tmp/test_file", O_RDONLY)) < 0) {
            perror("Open ./tmp/test_file");
            exit(EXIT_FAILURE);
        }

        do {
            ret = read(fd, buffer, 100);
        } while (ret);

        close(fd);
    }
}
