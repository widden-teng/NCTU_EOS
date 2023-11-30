#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_CONNECTIONS 5

pid_t childpid;

void childfunc(int new_socket) {
    // Redirect standard outputto the client socket
    // 若有STDIN_FILENO和 STDERR_FILENO ， client端的小火車的排版會亂掉
    dup2(new_socket, STDOUT_FILENO);


    // Close the original socket
    close(new_socket);

    // 在終端執行以下指令
    // Execute 'ls -l'
    execlp("sl", "sl", "-l", NULL);

    // If execlp fails
    perror("execlp");
    exit(EXIT_FAILURE);
}

void parentfunc(int new_socket) {
    close(new_socket);

    int status;
    pid_t finished_child;
    // Wait for the child process to finish(Zombie process )
    do {
        finished_child = waitpid(childpid, &status, 0);
        if (finished_child == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    printf("all child process is done\n");
}

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

    // Create a socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    // Force using socket address already in use!!!!
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Server listening on port %d...\n", port);

        childpid = fork();
        if (childpid >= 0) {
            if (childpid == 0) {
                // Child process
                close(server_fd); // Close the original socket in the child
                childfunc(new_socket);
            } else {
                // Parent process
                parentfunc(new_socket);
            }
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        printf("Connection handled.\n");
    }

    return 0;
}
