#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wand/MagickWand.h>


#define RESIZED_WIDTH 240
#define RESIZED_HEIGHT 180
#define SHM_KEY 1234

typedef struct {
    char pixels[240*180];
}ImageData;
#define SHMSZ sizeof(ImageData)

int main() {
    MagickWand *image_wand = NULL;

    // 初始化 MagickWand
    MagickWandGenesis();

    // 创建 MagickWand 对象
    image_wand = NewMagickWand();

    // 读取图像文件
    if (MagickReadImage(image_wand, "pic.jpg") == MagickFalse) {
        fprintf(stderr, "Unable to read image\n");
        exit(EXIT_FAILURE);
    }

    // 执行 resize 操作
    if (MagickResizeImage(image_wand, RESIZED_WIDTH, RESIZED_HEIGHT, LanczosFilter, 1.0) == MagickFalse) {
        fprintf(stderr, "Unable to resize image\n");
        exit(EXIT_FAILURE);
    }

    // 获取 resize 后的图像数据
    size_t resized_size;
    char *resized_pixels = (char *)MagickGetImageBlob(image_wand, &resized_size);

    // 创建共享内存段
    key_t key = ftok(".", 'x');

    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(SHM_KEY, SHMSZ, IPC_CREAT | 0666);

    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // 连接共享内存
    ImageData *shared_data = (ImageData *)shmat(shmid, NULL, 0);

    if ((intptr_t)shared_data == -1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // 将 resize 后的图像数据存入结构体
    memcpy(shared_data->pixels, resized_pixels, resized_size);

    // 分离共享内存
    if (shmdt(shared_data) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    // 释放资源
    if (resized_pixels != NULL) {
        free(resized_pixels);
    }

    // 清理资源
    if (image_wand != NULL) {
        DestroyMagickWand(image_wand);
    }

    // 终止 MagickWand
    MagickWandTerminus();

    printf("Image resized and stored in shared memory.\n");

    return 0;
}
