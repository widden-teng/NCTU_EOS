#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE] = {0};

    // Create a socket file descriptor for the client
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    server_addr.sin_family = AF_INET;
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

    // Main communication loop
    while (1) {
        // Receive a response from the server
        ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_received == -1) {
            perror("Read failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        if (bytes_received == 0) {
            printf("Server closed the connection\n");
            break;
        }

        buffer[bytes_received] = '\0';  // Ensure null-terminated string
        printf("%s", buffer);

        if(strcmp(buffer, "Please wait a few minutes...\n")==0){
            bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
        }

        // Get user input
        fgets(buffer, sizeof(buffer), stdin);

        // Check for exit command
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Exiting...\n");
            break;
        }

        // Send user input to the server
        if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
            perror("Send failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }

    // Close the socket
    if (close(client_socket) == -1) {
        perror("Close failed");
        exit(EXIT_FAILURE);
    }

    printf("Client closed\n");

    return 0;
}
