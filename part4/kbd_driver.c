#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nhom3");
MODULE_DESCRIPTION("Keyboard IRQ Handler with Workqueue");

#define DEVICE_NAME "kbd_log"
#define BUFFER_SIZE 1024
#define KBD_IRQ 1

static char buffer[BUFFER_SIZE];
static int head = 0;
static int tail = 0;
static spinlock_t buf_lock;
static int major;
static struct class *kbd_class;
static struct cdev kbd_cdev;

/* Workqueue */
static struct workqueue_struct *kbd_wq;
static DECLARE_WORK(kbd_work, NULL);
static unsigned char last_scancode = 0;

/* Chuyển scancode sang ASCII đơn giản */
static char scancode_to_ascii(unsigned char scancode) {
    static const char *keymap = "??1234567890-=??qwertyuiop[]??asdfghjkl;'`??zxcvbnm,./?";
    if (scancode < 0x3A)
        return keymap[scancode];
    return '?';
}

/* Hàm xử lý workqueue */
static void kbd_work_handler(struct work_struct *work) {
    char ascii;
    unsigned long flags;

    ascii = scancode_to_ascii(last_scancode);

    spin_lock_irqsave(&buf_lock, flags);
    buffer[head] = ascii;
    head = (head + 1) % BUFFER_SIZE;
    if (head == tail)
        tail = (tail + 1) % BUFFER_SIZE;
    spin_unlock_irqrestore(&buf_lock, flags);

    pr_info("Workqueue xu ly: Scancode = %x, ASCII = %c\n", last_scancode, ascii);
}

/* IRQ handler – chỉ giao việc */
static irqreturn_t irq_handler(int irq, void *dev_id) {
    unsigned char scancode = inb(0x60);

    if (!(scancode & 0x80)) {
        last_scancode = scancode;
        queue_work(kbd_wq, &kbd_work);
    }

    return IRQ_HANDLED;
}

static ssize_t kbd_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    unsigned long flags;
    char kbuf[BUFFER_SIZE];
    int copied = 0;

    spin_lock_irqsave(&buf_lock, flags);
    while (head != tail && copied < count) {
        kbuf[copied++] = buffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;
    }
    spin_unlock_irqrestore(&buf_lock, flags);

    if (copied == 0)
        return 0;

    if (copy_to_user(user_buf, kbuf, copied))
        return -EFAULT;

    return copied;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = kbd_read,
};

static int __init kbd_init(void) {
    dev_t dev;
    int ret;

    spin_lock_init(&buf_lock);
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    major = MAJOR(dev);
    pr_info("Allocated major number: %d\n", major);
    cdev_init(&kbd_cdev, &fops);
    cdev_add(&kbd_cdev, dev, 1);
    kbd_class = class_create("kbd_class");
    if (IS_ERR(kbd_class)) {
        pr_err("Không thể tạo class\n");
        return PTR_ERR(kbd_class);
    }

    device_create(kbd_class, NULL, dev, NULL, DEVICE_NAME);
    INIT_WORK(&kbd_work, kbd_work_handler);
    kbd_wq = create_singlethread_workqueue("kbd_wq");

    ret = request_irq(KBD_IRQ, irq_handler, IRQF_SHARED, "kbd_driver", (void *)(irq_handler));
    if (ret) {
        pr_err("Không thể đăng ký IRQ\n");
        return ret;
    }
    pr_info("kbd_driver đã được tải.\n");
    return 0;
}
static void __exit kbd_exit(void) {
    dev_t dev = MKDEV(major, 0);

    free_irq(KBD_IRQ, (void *)(irq_handler));
    flush_workqueue(kbd_wq);
    destroy_workqueue(kbd_wq);
    device_destroy(kbd_class, dev);
    class_destroy(kbd_class);
    cdev_del(&kbd_cdev);
    unregister_chrdev_region(dev, 1);

    pr_info("kbd_driver đã được gỡ bỏ.\n");
}
module_init(kbd_init);
module_exit(kbd_exit);
