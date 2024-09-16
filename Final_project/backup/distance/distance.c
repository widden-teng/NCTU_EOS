#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>

#define SHM_KEY 5555


// (大小為26 ＝ 8+8+4+4+2) timer也要一起改！！！！！！！！！！！！
typedef struct {
    char *front_camera_path;
    char *back_camera_path;
    short front_person_info[2];  
    short back_person_info[2];  
    short timestamp;     
}MyStruct;

MyStruct *sharedData;

int main() {

    int shmid = shmget(SHM_KEY, sizeof( MyStruct), IPC_CREAT | 0666);  // 創建共享內存區域

    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    sharedData = (MyStruct *)shmat(shmid, NULL, 0);  // 連接共享內存

    if ((void *)sharedData == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // 初始化自定義結構的成員
    sharedData->front_camera_path = strdup("/path/to/photo1.jpg");
    sharedData->back_camera_path = strdup("/path/to/photo2.jpg");

    sharedData->front_person_info[0] = 0;
    sharedData->front_person_info[1] = 0;

    sharedData->back_person_info[0] = 0;
    sharedData->back_person_info[1] = 0;
    sharedData->timestamp = 0;  

    // distance!!!!!!!!!!!!!!!!!.
    while(1){
        sleep(1);
    }

    // 記得在程式結束前釋放資源
    shmdt((void *)sharedData);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
