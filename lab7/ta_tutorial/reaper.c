/*
 * reaper.c
 * Demonstrate the work of process reaper
 *
 * 演示了父進程生成多個子進程，並使用 waitpid 函數和 SIGCHLD 信號處理程序來收集和等待子進程結束的情況。
 * 程式創建了多個子進程，每個子進程隨機休眠一段時間，然後退出。父進程使用 waitpid 在信號處理程序中非阻塞地等待子進程的結束，
 * 每當子進程結束時，信號處理程序就會被呼叫，顯示終止的子進程的PID。
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define FORKCHILD 5

volatile sig_atomic_t reaper_count = 0;

/* signal handler for SIGCHLD */
// Reaper 函數使用 waitpid 函數在非阻塞模式下等待任何子進程的結束。每當子進程結束時，
// 該信號處理程序被調用，顯示終止的子進程的PID，並將 reaper_count 增加。
void Reaper(int sig) {
    pid_t pid;
    int status;

    // 如果使用了 WNOHANG 選項，即 waitpid(-1, &status, WNOHANG)，它將以非阻塞模式工作。這意味著如果當前沒有結束的子進程，
    // 它會立即返回 0，而不會等待。如果有一個或多個子進程已經結束，它會返回第一個結束的子進程的 PID。
    // 當有一個以上的子進程在一次 SIGCHLD 中結束時，waitpid 可能一次就返回多個結束的子進程的信息。
    // 因此，使用 while 迴圈確保在 waitpid 返回的 PID 中處理所有已結束的子進程。
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child %d is terminated.\n", pid);
        reaper_count++;
    }
}

// 隨機休眠一段時間，然後退出
void ChildProcess() {
    int rand;

    /* rand a sleep time */
    srand(time(NULL));
    rand = random() % 10 + 2;

    printf("Child %d sleep %d sec.\n", getpid(), rand);
    sleep(rand);

    printf("Child %d exit.\n", getpid());
    exit(0);
}

int main(int argc, char *argv[]) {
    int cpid;
    int i;

    /* regist signal handler */
    signal(SIGCHLD, Reaper);

    /* fork child processes */
    for (i = 0; i < FORKCHILD; i++) {
        if ((cpid = fork()) > 0) /* parent */
            printf("Parent fork child process %d.\n", cpid);
        else /* child */
            ChildProcess();

        sleep(1);
    }

    /* wait all child exit */
    while (reaper_count != FORKCHILD)
        sleep(1);

    return 0;
}
