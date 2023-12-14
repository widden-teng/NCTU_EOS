#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>

#define MAX_BUFFER_SIZE 256

// global
int total_cookie = 0;
int total_cake = 0;
int total_tea = 0;
int total_boba = 0;
int total_fried_rice = 0;
int total_egg_drop_soup = 0;
int total_price = 0;
char initial_shop[50] = {0};
int distance = 0;



// 函數原型
void handleClient(int client_socket);
const char *get_shop_name(const char *item_name);
void update_num_price(const char *item_name, const int quantity);
char *show_current_order();
void reset_order();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_fd, new_socket;
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

    while (1) {
        // 接受新的連接
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Client connected\n");

        // 處理新的連接
        handleClient(new_socket);

        // 關閉與客戶端的 socket 連接
        close(new_socket);
        printf("Server closed connection with the client\n");
    }

    // 關閉伺服器端 socket
    close(server_fd);
    printf("Server closed\n");

    return 0;
}

// 定義函數 handleClient
void handleClient(int client_socket){

    char response[MAX_BUFFER_SIZE] = {0};
    char buffer[MAX_BUFFER_SIZE] = {0};
    char current_shop[50];
    bool first_order = true;
    char *item_name; 
    // snprintf(response, sizeof(response), "You can enter three commands: shop list, order, confirm and cancel\n");
    // snprintf(response, sizeof(response), "\n");
    // send(client_socket, response, strlen(response), 0);
    


    while (1)
    {
        memset(response, 0, sizeof(response));
        read(client_socket, buffer, sizeof(buffer));
        // printf("Received message from client: %s\n", buffer);
        if (strncmp(buffer, "shop list", 9) == 0) {
            // snprintf(response, sizeof(response), "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n");
            // send(client_socket, response, strlen(response), 0);
            send(client_socket, "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n", 256, 0);
            
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
                send(client_socket, order_history, 256, 0);
                free(order_history);
            }
        }
        else if (strncmp(buffer, "confirm", 7) == 0){

            if (first_order){
                snprintf(response, sizeof(response), "Please order some meals\n");
                send(client_socket, response, 256, 0);
            }
            else{
                snprintf(response, sizeof(response), "Please wait a few minutes...\n");

                send(client_socket, response, 256, 0);
                
                sleep(distance);

                
                snprintf(response, sizeof(response), "Delivery has arrived and you need to pay %d$\n", total_price);
                send(client_socket, response, 256, 0);

                reset_order();
                return;      
            }

        }
        else if (strncmp(buffer, "cancel", 6) == 0){
            reset_order();
            return;
        }
        
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
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d\n", "cake", total_cake);
        }

        // 動態記憶體的strlen 與 sizeof 使用方式不同, strlen會把中止字符算進來, sizeof則會很怪
        if(order_history[strlen(order_history)-1] == '|'){
            order_history[strlen(order_history)-1] = '\n';
        }
        
    } else if (strcmp(initial_shop, "Beverage shop") == 0) {
        if(total_tea != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d|", "tea", total_tea);
        }
        if(total_boba != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d\n", "boba", total_boba);
        }
        if(order_history[strlen(order_history)-1] == '|'){
            order_history[strlen(order_history)-1] = '\n';
        }     
        
    } else if (strcmp(initial_shop, "Diner") == 0) {
        if(total_fried_rice != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d|", "fried-rice", total_fried_rice);
        }
        if(total_egg_drop_soup != 0){
            snprintf(order_history + strlen(order_history), MAX_BUFFER_SIZE - strlen(order_history), "%s %d\n", "Egg-drop-soup", total_egg_drop_soup);
        }
        if(order_history[strlen(order_history)-1] == '|'){
            order_history[strlen(order_history)-1] = '\n';
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

