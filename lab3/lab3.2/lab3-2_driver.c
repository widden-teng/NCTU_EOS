/***************************************************************************
**
* \file lab3-2_deiver.c
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
#define GPIO_F (5)
#define GPIO_G (6)


dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
//File operation structure
/******************************************************/
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = etx_open,
    .release = etx_release,
    .write = etx_write,
};


static int gpio_pins[] = {GPIO_A, GPIO_B, GPIO_C, GPIO_D, GPIO_E, GPIO_F, GPIO_G};
static int num_pins = sizeof(gpio_pins) / sizeof(gpio_pins[0]);
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

static void set_display(int number) {
    for (int i = 0; i < num_pins; i++) {
        gpio_set_value(gpio_pins[i], SevenSegEncoder[number][i]);
    }
}

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file) {
    pr_info("Device File Opened...!!!\n");
    return 0;
}
/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file) {
    pr_info("Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    char rec_buf[10] = {0};

    if (copy_from_user(rec_buf, buf, len) > 0) {
        pr_err("ERROR: Not all the bytes have been copied from user\n");
    }

    pr_info("Write Function: Displaying number = %s\n", rec_buf);

    if (len != 1) {
        pr_err("Invalid input: Please provide a single-digit number\n");
        return -EINVAL;
    }

    int display_number = rec_buf[0] - '0';
    if (display_number >= 0 && display_number <= 9) {
        // 设置显示数字
        set_display(display_number);
    } else {
        pr_err("Invalid input: Please provide a number between 0 and 9\n");
        return -EINVAL;
    }

    return len;
}

static int __init etx_driver_init(void) {
    if ((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) < 0) {
        pr_err("Cannot allocate major number\n");
        goto r_unreg;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));
    cdev_init(&etx_cdev, &fops);
    if ((cdev_add(&etx_cdev, dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_del;
    }
    if ((dev_class = class_create(THIS_MODULE, "etx_class")) == NULL) {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }
    if ((device_create(dev_class, NULL, dev, NULL, "etx_device")) == NULL) {
        pr_err("Cannot create the Device \n");
        goto r_device;
    }

    int i;
    for (i = 0; i < num_pins; i++) {
        if (gpio_is_valid(gpio_pins[i]) == false) {
            pr_err("One or more GPIOs are not valid\n");
            goto r_device;
        }
        if (gpio_request(gpio_pins[i], "GPIO_PIN") < 0) {
            pr_err("ERROR: GPIO request\n");
            goto r_gpio;
        }
        gpio_direction_output(gpio_pins[i], 0);
        gpio_export(gpio_pins[i], false);
    }

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

r_gpio:
    for (i = 0; i < num_pins; i++) {
        gpio_free(gpio_pins[i]);
    }
r_device:
    device_destroy(dev_class, dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&etx_cdev);
r_unreg:
    unregister_chrdev_region(dev, 1);

    return -1;
}

static void __exit etx_driver_exit(void) {
    int i;
    for (i = 0; i < num_pins; i++) {
        gpio_unexport(gpio_pins[i]);
        gpio_free(gpio_pins[i]);
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
