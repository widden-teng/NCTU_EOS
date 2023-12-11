#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <unistd.h>
#include <arpa/inet.h>



int main(int argc, char *argv[]) {
    
    if (argc != 6) {
        printf("Usage: %s <ip> <port> <deposit/withdraw> <amount> <times>\n", argv[0]); 
        exit(1);  
    }
    
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    char *operation = argv[3];
    int amount = atoi(argv[4]);
    int times = atoi(argv[5]);
    char buffer[256] = {0};
    
    int client_socket;
    struct sockaddr_in server_addr; 
    
    // Create a socket file descriptor for the client
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set up server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    

    snprintf(buffer, sizeof(buffer), "%s %d %d", operation, amount, times);
    if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
        perror("Send failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    close(client_socket);

    return 0;
}