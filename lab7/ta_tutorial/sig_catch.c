/*
 * sig_catch.c
 * and use kill -USR1 <this_process>
 */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void handler(int signo, siginfo_t *info, void *context) {
    /* show the process ID sent signal */
    printf("Process (%d) sent SIGUSR1.\n", info->si_pid);
}

int main(int argc, char *argv[]) {
    struct sigaction my_action;

    /* register handler to SIGUSR1 */
    // 當進程收到SIGUSR1信號時，將調用handler函數，並通過siginfo_t結構體中的si_pid成員打印發送信號的進程ID。
    memset(&my_action, 0, sizeof(struct sigaction));
    my_action.sa_flags = SA_SIGINFO;
    my_action.sa_sigaction = handler;
    sigaction(SIGUSR1, &my_action, NULL);

    printf("Process (%d) is catching SIGUSR1 ...\n", getpid());
    sleep(10);
    printf("Done.\n");

    return 0;
}
