#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


int fd;
int led_flag = 0;


// Function to clear the terminal screen etx_Dev
void clearScreen() {
    // ANSI escape code to clear the screen
    printf("\033[2J\033[H");
}

int get_digitCount(int num){
    int digitCount = 0;
    do {
        num /= 10;
        digitCount++;
    } while (num != 0);
    return digitCount;
}

void write_to_dev(int shop_choice, int total_price){
    int distance ;
    if (shop_choice == 1) {
        distance = 3;
    } else if (shop_choice == 2) {
        distance = 5;
    } else if (shop_choice == 3) {
        distance = 8;
    }
    
    int digitCount = get_digitCount(total_price);
    int digit_iter = 0;
    int iter = (distance+1 > digitCount)? distance+1 : digitCount;

    char rec_buf[10] = {0};
    char buff_num[digitCount];
    char buff_distance[10];
    sprintf(buff_num, "%d", total_price);
    iter = iter*2;
    for(int i=0; i<iter; i++ ){

        if (i<digitCount){ 
            rec_buf[0] = buff_num[i]; 
        }
        else{
            rec_buf[0] = buff_num[digitCount-1];
        }

        if(led_flag%2 == 0){
            if (distance>=0){
                sprintf(buff_distance, "%d", distance);
                rec_buf[1] = (char)buff_distance[0];
                distance--;
            }
            else{
                rec_buf[1] = '0';
            }
        }


        // printf("rec_buf0 = %c\n", rec_buf[0]);
        // printf("rec_buf1 = %c\n", rec_buf[1]);
        usleep(500000);

        if( write(fd, rec_buf, 2 ) < 0) {
        perror("Failed to write to device file");
        }
        led_flag++;
    }

}

int main() {





    fd = open("/dev/etx_device", O_WRONLY);

    if (fd < 0) {
        perror("Failed to open device file");
        return 1;
    }

    while (1) {

        int choice;
        int total_cookie = 0;
        int total_cake = 0;
        int total_tea = 0;
        int total_boba = 0;
        int total_fried_rice = 0;
        int total_egg_drop_soup = 0;
        int total_price = 0;

        clearScreen();  // Clear the screen before printing the menu
        printf("1. shop list\n2. order\n");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                clearScreen();  // Clear the screen before printing the shop list
                printf("Dessert shop: 3km\nBeverage shop: 5km\nDiner: 8km\n");
                // printf("Press any key to return to the main menu...\n");
                getchar();
                getchar();
                break;
            case 2:
                clearScreen();  // Clear the screen before printing the order menu
                printf("Please choose from 1~3\n1. Dessert shop\n2. Beverage Shop\n3. Diner\n");
                int shop_choice;
                scanf("%d", &shop_choice);

                clearScreen();  // Clear the screen before printing the item menu
                printf("Please choose from 1~4\n");
                if (shop_choice == 1) {
                    printf("1. cookie: $60\n2. cake: $80\n3. confirm\n4. cancel\n");
                } else if (shop_choice == 2) {
                    printf("1. tea: $40\n2. boba: $70\n3. confirm\n4. cancel\n");
                } else if (shop_choice == 3) {
                    printf("1. fried rice: $120\n2. egg-drop soup: $50\n3. confirm\n4. cancel\n");
                }

                int item_choice, quantity;

                while (1) {
                    scanf("%d", &item_choice);
                    if (item_choice == 3) {
                        // Confirm order
                        if (total_cookie + total_cake + total_tea + total_boba + total_fried_rice + total_egg_drop_soup == 0) {
                            printf("Please order something before confirming!!!!!\n");
                        } else {
                            // printf("Please wait for a few minutes...\n");   
                            // printf("please pick up your meal\n");
                            if (shop_choice == 1) {
                                    printf("Dessert shop");
                            } else if (shop_choice == 2) {
                                printf("Beverage Shop");
                            } else if (shop_choice == 3) {
                                printf("Diner");
                            }
                            // printf("Total price is : %d\n", total_price);
                            write_to_dev(shop_choice, total_price);
                            getchar();
                            getchar();
                        }
                        break;
                    } else if (item_choice == 4) {
                        // Cancel order
                        printf("Order canceled\n");
                        total_cookie = 0;
                        total_cake = 0;
                        total_tea = 0;
                        total_boba = 0;
                        total_fried_rice = 0;
                        total_egg_drop_soup = 0;
                        total_price = 0;
                        break;
                    } else if (item_choice == 1 || item_choice == 2) {
                        printf("How many?\n");
                        scanf("%d", &quantity);

                        if (shop_choice == 1) {
                            if (item_choice == 1) {
                                total_cookie += quantity;
                                total_price += quantity * 60;
                            } else if (item_choice == 2) {
                                total_cake += quantity;
                                total_price += quantity * 80;
                            }
                        } else if (shop_choice == 2) {
                            if (item_choice == 1) {
                                total_tea += quantity;
                                total_price += quantity * 40;
                            } else if (item_choice == 2) {
                                total_boba += quantity;
                                total_price += quantity * 70;
                            }
                        } else if (shop_choice == 3) {
                            if (item_choice == 1) {
                                total_fried_rice += quantity;
                                total_price += quantity * 120;
                            } else if (item_choice == 2) {
                                total_egg_drop_soup += quantity;
                                total_price += quantity * 50;
                            }
                        }

                        clearScreen();  // Clear the screen before printing the next item menu
                        printf("Please choose from 1~4\n");
                        if (shop_choice == 1) {
                            printf("1. cookie: $60\n2. cake: $80\n3. confirm\n4. cancel\n");
                        } else if (shop_choice == 2) {
                            printf("1. tea: $40\n2. boba: $70\n3. confirm\n4. cancel\n");
                        } else if (shop_choice == 3) {
                            printf("1. fried rice: $120\n2. egg-drop soup: $50\n3. confirm\n4. cancel\n");
                        }
                    } else {
                        printf("Invalid choice\n");
                    }
                }
                break;
            default:
                printf("Invalid choice\n");
        }
    }

    return 0;
}
