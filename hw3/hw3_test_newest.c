#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <pthread.h>


/*share memory*/
struct total_result {
    int amount;
    char heading[10];
};
#define RESULT_SHMSZ sizeof(struct total_result) * 2
#define DELIVERY_SHMSZ sizeof(int) * 2
#define RESULT_SHNKEY 1234
#define DELIVERY_SHNKEY 5678
int result_shmid;
int delivery_shmid;
struct total_result *result_shm;
int *delivery_shm;

/*-------------------*/

/*Semaphore*/ 
#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */
#define MAX_BUFFER_SIZE 256
#define SEM_KEY 428361733 /* any long int */
#define SEM_VAL 1
int s;
/*-------------------*/


/*global for shop and order*/ 
int total_cookie = 0;
int total_cake = 0;
int total_tea = 0;
int total_boba = 0;
int total_fried_rice = 0;
int total_egg_drop_soup = 0;
int total_price = 0;
char initial_shop[50] = {0};
int distance = 0;
/*-------------------*/

/*socket and fork*/ 
int server_fd, client_socket;
int origin_parent;
/*-------------------*/

/*mutex*/ 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int rc = 0;
/*-------------------*/

/*function*/ 
void handleClient(void* arg);
const char *get_shop_name(const char *item_name);
void update_num_price(const char *item_name, const int quantity);
char *show_current_order();
void reset_order();
void stop_child(int signum);
void safe_close_routine(int signum);
int P(int s);
int V(int s);
void counting_time_CS();
int schedule_and_wait(int dis);
bool check_wating_time();
/*-------------------*/

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // signal
    signal(SIGCHLD, stop_child);
    signal(SIGINT, safe_close_routine);

    /*semophore setting*/
    s = semget(
        SEM_KEY, /* the unique name of the semaphore on the system */
        1,   /* we create an array of semaphores, but just need 1. */
        IPC_CREAT | IPC_EXCL | SEM_MODE);

    if (s < 0)
    {
        fprintf(stderr,
                "creation of semaphore %d failed: %s\n",
                SEM_KEY, strerror(errno));
        exit(1);
    }
    printf("Semaphore %d created\n", SEM_KEY);

    /* set semaphore (s[0]) value to initial value (val) */
    if (semctl(s, 0, SETVAL, SEM_VAL) < 0)
    {
        fprintf(stderr,
                "Unable to initialize semaphore: %s\n",
                strerror(errno));
        exit(0);
    }
    printf("Semaphore %d has been initialized to %d\n", SEM_KEY, SEM_VAL);
   
    /*-------------------------------------------------*/

    /*share memory setting for delivery*/ 
    /* Create the segment */
    if ((delivery_shmid = shmget(DELIVERY_SHNKEY, DELIVERY_SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    /* Now we attach the segment to our data space */
    if ((delivery_shm = shmat(delivery_shmid, NULL, 0)) == (int*) -1) {
        perror("shmat");
        exit(1);
    }
    // init delivery time
    delivery_shm[0] = 0;
    delivery_shm[1] = 0;

    /*-------------------------------------------------*/ 

    /*share memory setting for result.txt*/ 
    /* Create the segment */
    if ((result_shmid = shmget(RESULT_SHNKEY, RESULT_SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    /* Now we attach the segment to our data space */
    if ((result_shm = shmat(result_shmid, NULL, 0)) == (struct total_result *) -1) {
        perror("shmat");
        exit(1);
    }
    // init result time
    strcpy(result_shm[0].heading, "customer");
    result_shm[0].amount = 0;
    strcpy(result_shm[1].heading, "income");
    result_shm[1].amount = 0;

    

    /*-------------------------------------------------*/ 

    pid_t childpid;
    childpid = fork();    
    if (childpid >= 0) {
        if (childpid > 0) {
            // Parent process
            origin_parent = (int)getpid();
            counting_time_CS();
        } 
        else {
            // Child process

            /*  set socket  */
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
            /*-------------------------------------------------*/

            while (1) {
                // 接受新的連接
                if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) < 0) {
                    perror("Accept failed");
                    exit(EXIT_FAILURE);
                }

                printf("Client connected\n");

                handleClient(&client_socket);
            }
        }
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // 因為while(1)過不來
    // // 關閉伺服器端 socket
    // close(server_fd);
    // printf("Server closed\n");
    // return 0;
}

// 定義函數 handleClient
void handleClient(void* arg){
    int client_sockfd = *(int*)arg;

    // child process
    if (fork() == 0) {
        char response[MAX_BUFFER_SIZE] = {0};
        char buffer[MAX_BUFFER_SIZE] = {0};
        char current_shop[50];
        bool first_order = true;
        char *item_name;  

        while (1)
        {
            memset(response, 0, sizeof(response));
            read(client_sockfd, buffer, sizeof(buffer));

            if (strncmp(buffer, "shop list", 9) == 0) {
                send(client_sockfd, "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n", 256, 0);
                
            }
            else if (strncmp(buffer, "order", 5) == 0) {
                char *token = strtok(buffer, " ");
                item_name = strtok(NULL, " "); //product
                if (item_name != NULL) {
                    if (first_order) {
                        strcpy(initial_shop, get_shop_name(item_name));
                        first_order = false; 
                    }
                    if(strcmp(get_shop_name(item_name), initial_shop) == 0) {
                        token = strtok(NULL, " "); //number
                        if (token != NULL) {
                            int current_quantity = atoi(token);
                            update_num_price(item_name, current_quantity);
                        }
                        
                    }
                    char* order_history;
                    order_history = show_current_order();
                    send(client_sockfd, order_history, 256, 0);
                    free(order_history);
                }
            }
            else if (strncmp(buffer, "confirm", 7) == 0){

                if (first_order){
                    snprintf(response, sizeof(response), "Please order some meals");
                    send(client_sockfd, response, 256, 0);
                }
                else{
                    
                    //
                    bool return_flag = false;

                    // /* lock mutex */
                    // rc = pthread_mutex_lock(&mutex);
                    int shortest_time;
                    shortest_time = delivery_shm[0] < delivery_shm[1] ? delivery_shm[0]:delivery_shm[1];
                    shortest_time = shortest_time + distance;
                    if(shortest_time>30){
                        return_flag =  true;
                    }
                    else{
                        return_flag =  false;
                    }
                    

                    //

                    if(return_flag){
                        snprintf(response, sizeof(response), "Your delivery will take a long time, do you want to wait?");
                        send(client_sockfd, response, 256, 0);

                        read(client_sockfd, buffer, sizeof(buffer));
                        if (strncmp(buffer, "No", 2) == 0) {
                            reset_order();
                            // /* unlock mutex */
                            // rc = pthread_mutex_unlock(&mutex);
                            break; 
                        }
                        
                    }
                    // /* unlock mutex */
                    // rc = pthread_mutex_unlock(&mutex);
                    /*-------------------*/

                    snprintf(response, sizeof(response), "Please wait a few minutes...");
                    send(client_sockfd, response, 256, 0);
                    
                    int sleep_time = 0;
                    sleep_time = schedule_and_wait(distance);

                    sleep(sleep_time);

                    
                    snprintf(response, sizeof(response), "Delivery has arrived and you need to pay %d$", total_price);
                    send(client_sockfd, response, 256, 0);


                    /* lock mutex */
                    rc = pthread_mutex_lock(&mutex);
                    result_shm[0].amount++;
                    result_shm[1].amount = result_shm[1].amount + total_price;
                    /* unlock mutex */
                    rc = pthread_mutex_unlock(&mutex);
                    

                    reset_order();
                    break;      
                }

            }
            else if (strncmp(buffer, "cancel", 6) == 0){
                reset_order();
                break;
            }
            
        }


        // Close the socket for this client
        close(client_sockfd);
    }else {
        // parent process
        // 為了釋放與特定客戶端通訊相關的資源
        close(client_sockfd);
    }

}

// get the shop name of product
const char *get_shop_name(const char *item_name) {
    if (strcmp(item_name, "cookie") == 0 || strcmp(item_name, "cake") == 0) {
        distance = 3;
        return "Dessert shop";
    } else if (strcmp(item_name, "tea") == 0 || strcmp(item_name, "boba") == 0) {
        distance = 5;
        return "Beverage shop";
    } else if (strcmp(item_name, "fried-rice") == 0 || strcmp(item_name, "Egg-drop-soup") == 0) {
        distance = 8;
        return "Diner";
    } else {
        return "Unknown";
    }
}

// update number and total price by product
void update_num_price(const char *item_name, const int quantity) {
    int item_price = 0;
    if (strcmp(initial_shop, "Dessert shop") == 0) {
        if (strcmp(item_name, "cookie") == 0) {
            total_cookie = total_cookie + quantity;
            item_price =  60;
        } else if (strcmp(item_name, "cake") == 0) {
            total_cake = total_cake + quantity;
            item_price =  80;
        }
    } else if (strcmp(initial_shop, "Beverage shop") == 0) {
        if (strcmp(item_name, "tea") == 0) {
            total_tea = total_tea + quantity;
            item_price =  40;
        } else if (strcmp(item_name, "boba") == 0) {
            total_boba = total_boba + quantity;
            item_price =  70;
        }
    } else if (strcmp(initial_shop, "Diner") == 0) {
        if (strcmp(item_name, "fried-rice") == 0) {
            total_fried_rice = total_fried_rice + quantity;
            item_price =  120;
        } else if (strcmp(item_name, "Egg-drop-soup") == 0) {
            total_egg_drop_soup = total_egg_drop_soup + quantity;
            item_price =  50;
        }
    }
    total_price = total_price + item_price * quantity;
}

char *show_current_order(){
    
    char *order_history = (char *)malloc(MAX_BUFFER_SIZE );

    if (order_history == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    order_history[0] = '\0';

    if (strcmp(initial_shop, "Dessert shop") == 0) {
        if(total_cookie != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d|", "cookie", total_cookie);
            
        }
        if(total_cake != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d", "cake", total_cake);
        }

        // 動態記憶體的strlen 與 sizeof 使用方式不同, strlen會把中止字符算進來, sizeof則會很怪
        if(order_history[strlen(order_history)-1] == '|'){
            order_history[strlen(order_history)-1] = '\0';
        }
        
    } else if (strcmp(initial_shop, "Beverage shop") == 0) {
        if(total_tea != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d|", "tea", total_tea);
        }
        if(total_boba != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d", "boba", total_boba);
        }
        if(order_history[strlen(order_history)-1] == '|'){
            order_history[strlen(order_history)-1] = '\0';
        }     
        
    } else if (strcmp(initial_shop, "Diner") == 0) {
        if(total_fried_rice != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d|", "fried-rice", total_fried_rice);
        }
        if(total_egg_drop_soup != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d", "Egg-drop-soup", total_egg_drop_soup);
        }
        if(order_history[strlen(order_history)-1] == '|'){
            order_history[strlen(order_history)-1] = '\0';
        }              
    }
    return order_history;  
}

void reset_order(){
    total_cookie = 0;
    total_cake = 0;
    total_tea = 0;
    total_boba = 0;
    total_fried_rice = 0;
    total_egg_drop_soup = 0;
    total_price = 0;
    distance = 0;

}

void stop_child(int signum) {
    // waitpid 是為了等待並處理子進程的結束，以防止它們變成殭屍進程。
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// use to define my sigint
void safe_close_routine(int signum) {
    
    close(server_fd);

    
    if(origin_parent == (int)getpid()){
        /* remove semaphore */
       if (semctl(s, 0, IPC_RMID, 0) < 0)
        {
            fprintf(stderr, "unable to remove semaphore %d\n",
                    SEM_KEY);
            exit(1);
        } 
        /*---------------------------------*/ 
        
        /* create result.txt*/
        // 定義檔案指標
        FILE *filePointer;

        // 開啟檔案以進行寫入，如果檔案不存在則會被創建
        // 如果檔案存在，則會清空檔案內容
        filePointer = fopen("result.txt", "w");

        // 檢查檔案是否成功開啟
        if (filePointer == NULL) {
            printf("無法創建或開啟檔案。\n");
            exit(1); // 結束程式，返回錯誤碼
        }

        // 將資料寫入檔案
        fprintf(filePointer, "%s: %d\n%s: %d$", result_shm[0].heading, result_shm[0].amount, result_shm[1].heading, result_shm[1].amount);

        // 關閉檔案
        fclose(filePointer);

        printf("檔案已成功創建或清空並寫入。\n");
        /*---------------------------------------*/


    }      
    

    /* destroy mutex */
    pthread_mutex_destroy(&mutex);
    /*---------------------------------*/ 

    /* Detach the shared memory segment of delivery and  result*/
    shmdt(delivery_shm);
    shmdt(result_shm); 


    if(origin_parent == (int)getpid()){
        /*remove share memory*/
        /*remove delivery shm*/
        int retval;
        retval = shmctl(delivery_shmid, IPC_RMID, NULL);
        if (retval < 0) {
            fprintf(stderr, "Server removes delivery shared memory failed\n");
            exit(1);
        }

        /*remove result shm*/
        retval = shmctl(result_shmid, IPC_RMID, NULL);
        if (retval < 0) {
            fprintf(stderr, "Server removes result shared memory failed\n");
            exit(1);
        }
        /*------------------------------*/
    }

    // 結束程式
    exit(signum);            
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

void counting_time_CS(){

    
    int i ;
    while(1){
        /*critical section*/
        P(s);
        for (i=0; i<2; i++){
            if (delivery_shm[i]>0){
                printf("the %d shm is %d\n", i, delivery_shm[i]);
                delivery_shm[i] = delivery_shm[i]-1;
            }
        }
        V(s);
        /*-------------------*/
        sleep(1);
    }

}

// return true for both time > 30s
bool check_wating_time(){

    bool return_flag = false;

    // /*critical section*/
    // P(s);
    // int shortest_time;
    // shortest_time = delivery_shm[0] < delivery_shm[1] ? delivery_shm[0]:delivery_shm[1];
    // if(shortest_time>30){
    //     return_flag =  true;
    // }
    // else{
    //     return_flag =  false;
    // }
    // V(s);
    // /*-------------------*/

    int shortest_time;
    shortest_time = delivery_shm[0] < delivery_shm[1] ? delivery_shm[0]:delivery_shm[1];
    shortest_time = shortest_time + distance;
    if(shortest_time>30){
        return_flag =  true;
    }
    else{
        return_flag =  false;
    }

    return return_flag;

}

int schedule_and_wait(int dis){

    int return_time = 0;
    /*critical section*/
    P(s);
    if(delivery_shm[0] < delivery_shm[1]){
        delivery_shm[0] = delivery_shm[0] + dis;
        return_time = delivery_shm[0];
    }else{
        delivery_shm[1] = delivery_shm[1] + dis;
        return_time = delivery_shm[1];
    }
    V(s);
    /*-------------------*/

    return return_time;
}