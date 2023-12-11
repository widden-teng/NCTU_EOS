/* pipe.c
 *
 * child process reads the content of a file
 * and writes the content to the parent process through a pipe
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int pfd[2]; /* pfd[0] is the read end, pfd[1] is the write end */

void ChildProcess(char *path) {
    int fd;
    int ret;
    char buffer[100];

    /* close unused read end */
    close(pfd[0]);

    /* open file */
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("Open %s failed.\n", path);
        exit(EXIT_FAILURE);
    }

    /* read the file and write the content to the pipe */
    while (1) {
        /* read raw data from the file */
        ret = read(fd, buffer, 100);
        if (ret < 0) { /* error */
            perror("read()");
            exit(EXIT_FAILURE);
        } else if (ret == 0) { /* reach EOF */
            close(fd);      /* close the file */
            close(pfd[1]);   /* close the write end, so the reader sees EOF */
            exit(EXIT_SUCCESS);
        } else { /* write content to the pipe */
            write(pfd[1], buffer, ret);
        }
    }
}

void ParentProcess() {
    int ret;
    char buffer[100];

    /* close unused write end */
    close(pfd[1]);

    /* read data from the pipe until reaching EOF */
    while (1) {
        ret = read(pfd[0], buffer, 100);
        if (ret > 0) { /* print data to the screen */
            printf("%.*s", ret, buffer);
        } else if (ret == 0) { /* reach EOF */
            close(pfd[0]); /* close the read end */
            wait(NULL);
            exit(EXIT_SUCCESS);
        } else {
            perror("pipe read()");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    pid_t cpid;

    if (argc != 2) {
        fprintf(stderr, "%s: specify a file\n", argv[0]);
        exit(1);
    }

    /* create a pipe */
    if (pipe(pfd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* fork the child process */
    cpid = fork();
    if (cpid == -1) { /* error */
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0)
        ChildProcess(argv[1]);
    else
        ParentProcess();

    return 0;
}
