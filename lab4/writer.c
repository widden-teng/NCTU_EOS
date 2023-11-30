#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <name>\n", argv[0]);
        return 1;
    }

    int fd;
    fd = open("/dev/mydev", O_WRONLY);

    if (fd < 0) {
        perror("Failed to open device file");
        return 1;
    }

    char *name = argv[1];
    int name_length = strlen(name);

    for (int i = 0; i < name_length; i++) {
        if ((name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= 'a' && name[i] <= 'z')) {
            char letter[2] = {name[i], '\0'};
            if (write(fd, letter, 1) < 0) {
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
