#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
void decToBinary(int n, char *binaryStr) {
    for (int i = 3; i >= 0; i--) {
        binaryStr[i] = (n & (1 << (3 - i))) ? '1' : '0';
    }
    binaryStr[4] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <decimal_number_1> <decimal_number_2> ...\n", argv[0]);
        return 1;
    }

    int fd;
    fd = open("/dev/etx_device", O_WRONLY);

    if (fd < 0) {
        perror("Failed to open device file");
        return 1;
    }

    // for (int i = 1; i < argc; i++) {
    //     int decimal_number = atoi(argv[i]);
    //     char binary_str[5];
    //     decToBinary(decimal_number, binary_str);

    //     // Write the binary string to the device
    //     if (write(fd, binary_str, 4) < 0) {
    //         perror("Failed to write to device file");
    //         close(fd);
    //         return 1;
    //     }

    //     sleep(1); // Wait for 1 second before sending the next number
    // }

    char *sequence = argv[1]; // 获取输入的一连串数字
    for (int i = 0; i < strlen(sequence); i++) {
        if (sequence[i] >= '0' && sequence[i] <= '9') {
            int decimal_number = sequence[i] - '0'; // 将字符转换为整数
            char binary_str[5]; // 包括字符串终止符'\0'
            decToBinary(decimal_number, binary_str);

            // Write the binary string to the device
            if (write(fd, binary_str, 4) < 0) {
                perror("Failed to write to device file");
                close(fd);
                return 1;
            }

            sleep(1); // Wait for 1 second before sending the next number
        }
    }

    close(fd);
    return 0;
}
