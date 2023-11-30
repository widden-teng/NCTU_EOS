#include <stdio.h>      // fprintf(), perror()
#include <stdlib.h>     // exit()
#include <string.h>     // memset()
#include <signal.h>    // signal()
#include <fcntl.h>     // open()
#include <unistd.h>    // read(), write(), close()

#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // htons()

int connfd, fd;

void sigint_handler(int signo) {
    close(fd);
    close(connfd);
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Usage: ./reader <server_ip> <port> <device_path>");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sigint_handler);

    // 建立socket：接下來，使用 socket 函數建立一個socket，並將該socket的描述符賦值給 connfd。這個socket用於與遠程服務器進行通信。
    if ((connfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    // 初始化服務器地址結構：然後，初始化 struct sockaddr_in 結構 cli_addr，並設置協議家族為 AF_INET。
    // 它還設置服務器的IP地址和端口號，這些信息來自命令行參數。端口號會轉換成網絡字節序（big-endian）。
    struct sockaddr_in cli_addr;
    memset(&cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = inet_addr(argv[1]);
    cli_addr.sin_port = htons((u_short)atoi(argv[2]));

    // 建立連接：使用 connect 函數建立到遠程服務器的連接，將連接socket connfd 連接到服務器地址。如果連接失敗，程式會顯示相應的錯誤消息並退出。
    if(connect(connfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    if((fd = open(argv[3], O_RDWR)) < 0) {
        perror(argv[3]);
        exit(EXIT_FAILURE);
    }

    int ret;
    char buf[16] = {};

    while (1) {
        if((ret = read(fd, buf, sizeof(buf))) == -1) {
            perror("read()");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < 16; ++i) {
            printf("%c ", buf[i]);
        }
        printf("\n");

        if(write(connfd, buf, 16) == -1) {
            perror("write()");
            exit(EXIT_FAILURE);
        }

        sleep(1);
    }

    close(fd);
    close(connfd);
    return 0;
}
