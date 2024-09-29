#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define BCM2711_PERI_BASE        0xFE000000
#define GPIO_BASE                (BCM2711_PERI_BASE + 0x200000) // 사용하려는 GPIO 물리 주소

#define GPIO_PIN_BCM_BUZZER      17
#define GPIO_PIN_BCM_BUZZER_LED  27

static volatile unsigned int *gpio;

#define GPIO_IN(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define GPIO_OUT(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)

// 주요 번호
#define DEVICE_NAME "buzzer_dev"
#define MAJOR_NUM 230

static int buzzer_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "buzzer_dev: Device opened\n");
    return 0;
}

static ssize_t buzzer_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
    int state;
    if (copy_from_user(&state, buffer, sizeof(int))) {
        printk(KERN_ERR "buzzer_dev: Failed to get data from user\n");
        return -EFAULT;
    }
    if (state)
        GPIO_SET = 1 << GPIO_PIN_BCM_BUZZER | 1 << GPIO_PIN_BCM_BUZZER_LED;
    else
        GPIO_CLR = 1 << GPIO_PIN_BCM_BUZZER | 1 << GPIO_PIN_BCM_BUZZER_LED;

    printk(KERN_INFO "buzzer_dev: Written value %d to GPIO\n", state);
    return sizeof(int);
}

static int buzzer_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "buzzer_dev: Device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = buzzer_open,
    .write = buzzer_write,
    .release = buzzer_release,
};

static int __init buzzer_init(void) {
    int result;

    printk(KERN_INFO "buzzer_dev: Initializing\n");
    gpio = (unsigned int *)ioremap(GPIO_BASE, 0xB4);

    if (!gpio) {
        printk(KERN_ERR "buzzer_dev: Failed to map GPIO\n");
        return -EBUSY;
    }

    GPIO_IN(GPIO_PIN_BCM_BUZZER);
    GPIO_OUT(GPIO_PIN_BCM_BUZZER);
    GPIO_CLR = 1 << GPIO_PIN_BCM_BUZZER;

    GPIO_IN(GPIO_PIN_BCM_BUZZER_LED);
    GPIO_OUT(GPIO_PIN_BCM_BUZZER_LED);
    GPIO_CLR = 1 << GPIO_PIN_BCM_BUZZER_LED;

    result = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
    if (result < 0) {
        printk(KERN_ERR "buzzer_dev: Failed to register character device\n");
        iounmap(gpio);
        return result;
    }

    printk(KERN_INFO "buzzer_dev: Module loaded successfully\n");
    return 0;
}

static void __exit buzzer_exit(void) {
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    GPIO_CLR = 1 << GPIO_PIN_BCM_BUZZER | 1 << GPIO_PIN_BCM_BUZZER_LED;
    iounmap(gpio);
    printk(KERN_INFO "buzzer_dev: Module unloaded\n");
}

module_init(buzzer_init);
module_exit(buzzer_exit);

MODULE_LICENSE("GPL");

