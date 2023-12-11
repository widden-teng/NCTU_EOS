#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/sem.h>

#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */
#define NUMTHREADS 10
#define MAX_BUFFER_SIZE 256
#define KEY 428361733 /* any long int */
#define VAL 1

#define checkResults(string, val)                        \
    {                                                    \
        if (val) {                                       \
            printf("Failed with %d at %s", val, string); \
            exit(1);                                     \
        }                                                \
    }


// global variable
pthread_t thread[NUMTHREADS];
int rc = 0;
int tidx = 0;
int server_fd, client_socket;
int total = 0;
int user_num =0;
int s; //for Semophore

// use to define my sigint
void safe_close_routine(int signum) {
    
    printf("Main thread waits for threads to complete and release their resources\n");
    for (int i = 0; i < tidx; ++i) {
        rc = pthread_join(thread[i], NULL);
        checkResults("pthread_join()\n", rc);
    }
    close(server_fd);

    /* remove semaphore */
    if (semctl(s, 0, IPC_RMID, 0) < 0)
    {
        fprintf(stderr, "unable to remove semaphore %d\n",
                KEY);
        exit(1);
    }

    printf("Semaphore %d has been removed\n", KEY);
    exit(signum);            // 結束程式
}

/* P () - returns 0 if OK; -1 if there was a problem */
int P(int s)
{
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0;   /* access the 1st (and only) sem in the array */
    sop.sem_op = -1;   /* wait..*/
    sop.sem_flg = 0;   /* no special options needed */

    if (semop(s, &sop, 1) < 0)
    {
        fprintf(stderr, "P(): semop failed: %s\n", strerror(errno));
        return -1;
    }
    else
    {
        return 0;
    }
}

/* V() - returns 0 if OK; -1 if there was a problem */
int V(int s)
{
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0;   /* the 1st (and only) sem in the array */
    sop.sem_op = 1;    /* signal */
    sop.sem_flg = 0;   /* no special options needed */

    if (semop(s, &sop, 1) < 0)
    {
        fprintf(stderr, "V(): semop failed: %s\n", strerror(errno));
        return -1;
    }
    else
    {
        return 0;
    }
}


void* handle_client(void* arg){
    int client_sockfd = *(int*)arg;
    char buffer[MAX_BUFFER_SIZE] = {0};
    int oper = 1;
    int rc;
    char operation[20];  // 存放字符串类型
    int amout;          // 存放整数类型
    int times;          // 存放整数类型

    read(client_sockfd, buffer, sizeof(buffer));
    int result = sscanf(buffer, "%19s %d %d", operation, &amout, &times);

    if (strncmp(buffer, "withdraw", 7) == 0) 
        oper = -1;

    // 等4個client都連到(為了同時去做critical section
    do{

    }while(user_num !=4);

    int i ;
    for(i=0; i<times; i++){
        P(s);
        /**************** Critical Section *****************/       
        total = total + oper * amout;
        printf("After %s: %d\n", operation,total);
        /**************** Critical Section *****************/
        V(s);       
    }
   
    // Close the socket for this client
    close(client_sockfd);

}

int main(int argc, char **argv) {
    
    
    /* socket setting*/
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // signal
    signal(SIGINT, safe_close_routine);

    //semophore setting
    s = semget(
        KEY, /* the unique name of the semaphore on the system */
        1,   /* we create an array of semaphores, but just need 1. */
        IPC_CREAT | IPC_EXCL | SEM_MODE);

    if (s < 0)
    {
        fprintf(stderr,
                "creation of semaphore %d failed: %s\n",
                KEY, strerror(errno));
        exit(1);
    }
    printf("Semaphore %d created\n", KEY);

    /* set semaphore (s[0]) value to initial value (val) */
    if (semctl(s, 0, SETVAL, VAL) < 0)
    {
        fprintf(stderr,
                "Unable to initialize semaphore: %s\n",
                strerror(errno));
        exit(0);
    }

    printf("Semaphore %d has been initialized to %d\n", KEY, VAL);

    /*這邊為測試, 同個program不需要!!!*/ 
    /* find semaphore */
    /**如果指定的金鑰 key 所標識的信號量集已經存在，則返回其標識符。
     * 如果信號量集不存在，則返回 -1，並設置 errno 以指示錯誤
     * */
    s = semget(KEY, 1, 0);
    if (s < 0)
    {
        fprintf(stderr,
                "cannot find semaphore %d: %s\n",
                KEY, strerror(errno));
        exit(1);
    }


    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    int addrlen = sizeof(server_addr);

    // 創建一個 socket 檔案描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 設置 socket 選項
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 設置伺服器地址結構
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // 將 socket 綁定到指定的 port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 監聽傳入的連接
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);
    /*---------------------------------------------------------*/  
    
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("Client connected\n");

        // create thread
        rc = pthread_create(&thread[tidx++], NULL, handle_client, &client_socket);
        checkResults("pthread_create()\n", rc);
        user_num++;
    }

    /* wait for threads to complete */
    printf("Main thread waits for threads to complete and release their resources\n");
    for (int i = 0; i < tidx; ++i) {
        rc = pthread_join(thread[i], NULL);
        checkResults("pthread_join()\n", rc);
    }


    close(server_fd);
    printf("Main thread completed\n");

    return 0;
}
