
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// 使用sig_atomic_t類型的全局變數sigusr1_count來保存收到SIGUSR1信號的次數，這樣可以確保在多線程環境下進行原子操作
sig_atomic_t sigusr1_count = 0;

void handler(int signal_number) {
    ++sigusr1_count; /* add one, protected atomic action */
}

int main() {
    struct sigaction sa;
    struct timespec req;
    int retval;


    /* register handler to SIGUSR1 */
    // memset(&sa, 0, sizeof(sa));）這表示沒定義的其他參數使用默認的參數值。
    // 使用sigaction函數將handler函數註冊到SIGUSR1信號，這樣每當收到SIGUSR1信號時，將調用handler函數，並在其中將sigusr1_count增加一次。
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigaction(SIGUSR1, &sa, NULL);

    printf("catching SIGUSR1 ...\n");



    // alert !!!!!!!!!!!!!!!!!!!!!!!
    while(1){



    }


    return 0;
}
