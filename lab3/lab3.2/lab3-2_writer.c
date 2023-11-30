#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int fd;
    fd = open("/dev/etx_device", O_WRONLY);

    if (fd < 0) {
        perror("Failed to open device file");
        return 1;
    }

    char *sequence = argv[1];
    for (int i = 0; i < strlen(sequence); i++) {
        if (sequence[i] >= '0' && sequence[i] <= '9') {
            char digit[2] = {sequence[i], '\0'};
            if (write(fd, digit, 1) < 0) {
                perror("Failed to write to device file");
                close(fd);
                return 1;
            }
            sleep(1);
        }
    }

    close(fd);
    return 0;
}
