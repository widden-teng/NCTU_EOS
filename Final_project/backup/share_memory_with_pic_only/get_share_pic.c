#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wand/MagickWand.h>

#define SHM_KEY 1234

typedef struct {
    char pixels[240*180];
}ImageData;
#define SHMSZ sizeof(ImageData)

int main() {
    // 创建 MagickWand 对象
    MagickWand *image_wand = NewMagickWand();

    // 创建共享内存段
    key_t key = ftok(".", 'x');

    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(SHM_KEY, SHMSZ, 0666);

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

    // 将共享内存中的图像数据读取到 MagickWand 中
    MagickReadImageBlob(image_wand, shared_data->pixels, SHMSZ );

    // // 显示图像
    // MagickDisplayImage(image_wand);

    // 保存图像
    if (MagickWriteImage(image_wand, "output_displayed.jpg") == MagickFalse) {
        fprintf(stderr, "Unable to write image\n");
        exit(EXIT_FAILURE);
    }

    /* Detach the shared memory segment*/
    int retval;
    shmdt(shared_data);
    /* Remove share memory*/ 
    retval = shmctl(shmid, IPC_RMID, NULL);
    if (retval < 0) {
        fprintf(stderr, "Server removes delivery shared memory failed\n");
        exit(1);
    }
    /*------------------------------*/

    // 清理资源
    if (image_wand != NULL) {
        DestroyMagickWand(image_wand);
    }

    printf("Image displayed and saved.\n");

    return 0;
}
