#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>



#define checkResults(string, val)                        \
    {                                                    \
        if (val) {                                       \
            printf("Failed with %d at %s", val, string); \
            exit(1);                                     \
        }                                                \
    }

#define NUMTHREADS 10
#define MAX_BUFFER_SIZE 256


// global variable
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread[NUMTHREADS];
int rc = 0;
int tidx = 0;
int server_fd, client_socket;
int total = 0;
int user_num =0;

// use to define my sigint
void safe_close_routine(int signum) {
    
    printf("Main thread waits for threads to complete and release their resources\n");
    for (int i = 0; i < tidx; ++i) {
        rc = pthread_join(thread[i], NULL);
        checkResults("pthread_join()\n", rc);
    }
    close(server_fd);

    /* destroy mutex */
    pthread_mutex_destroy(&mutex);
    printf("Main thread cleans up mutex\n");
    exit(signum);            // 結束程式
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
        
        /* lock mutex */
        rc = pthread_mutex_lock(&mutex);
        checkResults("pthread_mutex_lock()\n", rc);
        /**************** Critical Section *****************/       
        total = total + oper * amout;
        printf("After %s: %d\n", operation,total);
        /**************** Critical Section *****************/

        /* unlock mutex */
        rc = pthread_mutex_unlock(&mutex);
        checkResults("pthread_mutex_unlock()\n", rc); 
       
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

    /* destroy mutex */
    printf("Main thread cleans up mutex\n");
    rc = pthread_mutex_destroy(&mutex);

    close(server_fd);
    printf("Main thread completed\n");

    return 0;
}
