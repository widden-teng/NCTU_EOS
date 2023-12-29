/* rmsem.c
 *
 * This program destroys a semaphore. The user should pass a number
 * to be used as the semaphore key.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>

int main(int argc, char **argv)
{
    int s;
    long int key;

    if (argc != 2)
    {
        fprintf(stderr, "%s: specify a key (long)\n", argv[0]);
        exit(1);
    }

    /* get values from command line */
    if (sscanf(argv[1], "%ld", &key) != 1)
    {
        /* convert arg to long integer */
        fprintf(stderr,
                "%s: argument #1 must be a long integer\n", argv[0]);
        exit(1);
    }

    /* find semaphore */
    /**如果指定的金鑰 key 所標識的信號量集已經存在，則返回其標識符。
     * 如果信號量集不存在，則返回 -1，並設置 errno 以指示錯誤
     * */
    s = semget(key, 1, 0);
    if (s < 0)
    {
        fprintf(stderr, "%s: failed to find semaphore %ld: %s\n",
                argv[0], key, strerror(errno));
        exit(1);
    }

    printf("Semaphore %ld found\n", key);

    /* remove semaphore */
    if (semctl(s, 0, IPC_RMID, 0) < 0)
    {
        fprintf(stderr, "%s: unable to remove semaphore %ld\n",
                argv[0], key);
        exit(1);
    }

    printf("Semaphore %ld has been removed\n", key);

    return 0;
}
