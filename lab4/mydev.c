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

MODULE_LICENSE("GPL");

#define MAJOR_NUM 271
#define DEVICE_NAME "mydev"

static int major_num;
static char previous_letter = 'A';

// 定义 seg_for_c 数组...

static struct class *mydev_class;   // 声明设备类
static struct device *mydev_device; // 声明设备
static struct cdev my_cdev;
static dev_t my_dev;


static char seg_for_c[28][16] = {
    {1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1},  // A
    {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1},  // b
    {1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // C
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0 ,1},  // d
    {1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},  // E
    {1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1},  // F
    {1,0,0,1,1,1,1,1,0,0,0,1,0,0,0,0},  // G
    {0,0,1,1,0,0,1,1,0,0,0,1,0,0,0,1},  // H
    {1,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0},  // I
    {1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0},  // J
    {0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0},  // K
    {0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0},  // L
    {0,0,1,1,0,0,1,1,1,0,1,0,0,0,0,0},  // M
    {0,0,1,1,0,0,1,1,1,0,0,0,1,0,0,0},  // N
    {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0},  // O
    {1,0,0,0,0,0,1,1,0,1,0,0,0,0,0,1},  // P
    {0,1,1,1,0,0,0,0,0,1,0,1,0,0,0,0},  // q
    {1,1,1,0,0,0,1,1,0,0,0,1,1,0,0,1},  // R
    {1,1,0,1,1,1,0,1,0,0,0,1,0,0,0,1},  // S
    {1,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0},  // T
    {0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0},  // U
    {0,0,0,0,0,0,1,1,0,0,1,0,0,0,1,0},  // V
    {0,0,1,1,0,0,1,1,0,0,0,0,1,0,1,0},  // W
    {0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0},  // X
    {0,0,0,0,0,0,0,0,1,0,1,0,0,1,0,0},  // Y
    {1,1,0,0,1,1,0,0,0,0,1,0,0,0,1,0},  // Z
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}   // Empty
};

static ssize_t my_read(struct file *filep, char *buf, size_t count, loff_t *fpos) {
    printk("Call read\n");

    if (count != sizeof(seg_for_c[27])) {
        return -EINVAL;
    }

    int index = previous_letter - 'A';

    if (copy_to_user(buf, seg_for_c[index], count) != 0) {
        return -EFAULT;
    }

    return count;
}

static ssize_t my_write(struct file *filep, const char *buf, size_t count, loff_t *fpos) {
    printk("Call write\n");

    if (count == 1) {
        if (copy_from_user(&previous_letter, buf, 1) != 0) {
            return -EFAULT;
        }
    } else {
        pr_err("Invalid input: Please provide one char\n");
        return -EINVAL;
    }

    return count;
}

static int my_open(struct inode *inode, struct file *filep) {
    printk("Call open\n");
    return 0;
}

struct file_operations my_fops = {
    read: my_read,
    write: my_write,
    open: my_open,
};

static int my_init(void) {
    printk("Call init\n");

    my_dev = MKDEV(MAJOR_NUM, 0);

    if (register_chrdev_region(my_dev, 1, DEVICE_NAME) < 0) {
        printk("Failed to register a region\n");
        return -EIO;
    }

    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;

    if (cdev_add(&my_cdev, my_dev, 1) < 0) {
        printk("Failed to add the character device\n");
        unregister_chrdev_region(my_dev, 1);
        return -EIO;
    }

    printk("My device is started and the major is %d\n", MAJOR(my_dev));

    return 0;
}

static void my_exit(void) {
    cdev_del(&my_cdev);
    unregister_chrdev_region(my_dev, 1);

    printk("Call exit\n");
}

module_init(my_init);
module_exit(my_exit);
