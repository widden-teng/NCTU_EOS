/***************************************************************************
**
* \file lab3-1_deiver.c
* \details Simple GPIO driver explanation
* \author EmbeTronicX
* \Tested with Linux raspberrypi 5.4.51-v7l+
******************************************************************************
*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/gpio.h> //GPIO
//LED is connected to this GPIO

// define GPIO pin
#define GPIO_A (21)
#define GPIO_B (20)
#define GPIO_C (16)
#define GPIO_D (12)
#define GPIO_E (1)
#define GPIO_F (26)
#define GPIO_G (19)

#define GPIO_led8 (2)
#define GPIO_led7 (3)
#define GPIO_led6 (4)
#define GPIO_led5 (17)
#define GPIO_led4 (27)
#define GPIO_led3 (22)
#define GPIO_led2 (10)
#define GPIO_led1 (9)


dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_write(struct file *filp,
 const char *buf, size_t len, loff_t * off);
/******************************************************/
//File operation structure
static struct file_operations fops =
{
 .owner = THIS_MODULE,
 .write = etx_write,
 .open = etx_open,
 .release = etx_release,
};


static int seg7_pins[] = {GPIO_A, GPIO_B, GPIO_C, GPIO_D, GPIO_E, GPIO_F, GPIO_G};
static int led_pins[] = {GPIO_led1, GPIO_led2, GPIO_led3, GPIO_led4, GPIO_led5, GPIO_led6, GPIO_led7, GPIO_led8};
static int num_seg7_pins = sizeof(seg7_pins) / sizeof(seg7_pins[0]);
static int num_leds_pins = sizeof(led_pins) / sizeof(led_pins[0]);
static int SevenSegEncoder[10][7] = {
    {1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 0, 0, 0, 0},
    {1, 1, 0, 1, 1, 0, 1},
    {1, 1, 1, 1, 0, 0, 1},
    {0, 1, 1, 0, 0, 1, 1},
    {1, 0, 1, 1, 0, 1, 1},
    {0, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 0, 0, 1, 1}
};

static void set_7seg_display(int number) {
    for (int i = 0; i < num_seg7_pins; i++) {
        gpio_set_value(seg7_pins[i], SevenSegEncoder[number][i]);
    }
}

static void set_led_display(int number) {

    for (int i = 0; i < 8; i++) {
        gpio_set_value(led_pins[i], ((i+1)<=number) ? 1 : 0);
    }
}

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
 pr_info("Device File Opened...!!!\n");
 return 0;
}
/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
 pr_info("Device File Closed...!!!\n");
 return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp,
 const char __user *buf, size_t len, loff_t *off)
{
    char rec_buf[10] = {0};

    if( copy_from_user( rec_buf, buf, len ) > 0) {
    pr_err("ERROR: Not all the bytes have been copied from user\n");
    }

    pr_info("Write Function: Displaying 7seg and led = %s\n", rec_buf);

        if (len != 2) {
        pr_err("Invalid input: Please provide a single-digit number\n");
        return -EINVAL;
    }

    int display_7seg_number = rec_buf[0] - '0';
    int display_led_number = rec_buf[1] - '0';
    if (display_7seg_number >= 0 && display_7seg_number <= 9) {
        set_7seg_display(display_7seg_number);
        set_led_display(display_led_number);
    } else {
        pr_err("Invalid input: Please provide a number between 0 and 9\n");
        return -EINVAL;
    }

    return len;
}
/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
    pr_err("Cannot allocate major number\n");
    goto r_unreg;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);
    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0){
    pr_err("Cannot add the device to the system\n");
    goto r_del;
    }
    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
    pr_err("Cannot create the struct class\n");
    goto r_class;
    }
    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
    pr_err( "Cannot create the Device \n");
    goto r_device;
    }

    //Checking the GPIO is valid or not 
    int i;
    for (i = 0; i < num_seg7_pins; i++) {
        if (gpio_is_valid(seg7_pins[i]) == false) {
            pr_err("One or more GPIOs are not valid\n");
            goto r_device;
        }
        if (gpio_request(seg7_pins[i], "GPIO_PIN") < 0) {
            pr_err("ERROR: GPIO request\n");
            goto r_gpio;
        }
        gpio_direction_output(seg7_pins[i], 0);
        gpio_export(seg7_pins[i], false);
    }

    for (i = 0; i < num_leds_pins; i++) {
        if (gpio_is_valid(led_pins[i]) == false) {
            pr_err("One or more GPIOs are not valid\n");
            goto r_device;
        }
        if (gpio_request(led_pins[i], "GPIO_PIN") < 0) {
            pr_err("ERROR: GPIO request\n");
            goto r_gpio;
        }
        gpio_direction_output(led_pins[i], 0);
        gpio_export(led_pins[i], false);
    }

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

    r_gpio:
    for (i = 0; i < num_seg7_pins; i++) {
        gpio_free(seg7_pins[i]);
    }
    for (i = 0; i < num_leds_pins; i++) {
        gpio_free(led_pins[i]);
    }

    r_device:
    device_destroy(dev_class,dev);
    r_class:
    class_destroy(dev_class);
    r_del:
    cdev_del(&etx_cdev);
    r_unreg:
    unregister_chrdev_region(dev,1);

    return -1;
}
/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
    int i;
    for (i = 0; i < num_seg7_pins; i++) {
        gpio_unexport(seg7_pins[i]);
        gpio_free(seg7_pins[i]);
    }
    for (i = 0; i < num_leds_pins; i++) {
        gpio_unexport(led_pins[i]);
        gpio_free(led_pins[i]);
    }

    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!\n");
}
module_init(etx_driver_init);
module_exit(etx_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
MODULE_VERSION("1.32");
