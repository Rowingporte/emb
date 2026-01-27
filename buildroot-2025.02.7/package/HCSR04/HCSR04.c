#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/of.h>              
#include <linux/mod_devicetable.h> 
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>

struct hcsr04_data {
    struct gpio_desc *trig;
    struct gpio_desc *echo;
    struct mutex lock;
    struct miscdevice miscdev;
};

static s64 get_raw_duration_us(struct hcsr04_data *data) {
    ktime_t start, end;
    int timeout;

    gpiod_set_value(data->trig, 1);
    udelay(10);
    gpiod_set_value(data->trig, 0);

    timeout = 1000;
    while (gpiod_get_value(data->echo) == 0 && timeout--) udelay(1);
    if (timeout <= 0) return -ETIMEDOUT;
    
    start = ktime_get();

    timeout = 30000;
    while (gpiod_get_value(data->echo) == 1 && timeout--) udelay(1);
    if (timeout <= 0) return -ETIMEDOUT;

    end = ktime_get();
    return ktime_to_us(ktime_sub(end, start));
}

static ssize_t hcsr04_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct hcsr04_data *data = container_of(file->private_data, struct hcsr04_data, miscdev);
    char buffer[32];
    int len;
    s64 duration;

    if (*ppos > 0) return 0;

    if (mutex_lock_interruptible(&data->lock))
        return -ERESTARTSYS;

    duration = get_raw_duration_us(data);
    mutex_unlock(&data->lock);

    if (duration < 0) return -EIO;

    len = scnprintf(buffer, sizeof(buffer), "%lld\n", duration);
    if (copy_to_user(buf, buffer, len)) return -EFAULT;
    
    *ppos = len;
    return len;
}

static const struct file_operations hcsr04_fops = {
    .owner = THIS_MODULE,
    .read = hcsr04_read,
    .open = simple_open,
};

static int hcsr04_probe(struct platform_device *pdev) {
    struct hcsr04_data *data;
    int ret;

    dev_info(&pdev->dev, "HCSR04: Probing device\n");

    data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
    if (!data) return -ENOMEM;

    data->trig = devm_gpiod_get(&pdev->dev, "trig", GPIOD_OUT_LOW);
    data->echo = devm_gpiod_get(&pdev->dev, "echo", GPIOD_IN);

    if (IS_ERR(data->trig) || IS_ERR(data->echo)) {
        dev_err(&pdev->dev, "Failed to get GPIOs from DTS\n");
        return -EINVAL;
    }

    mutex_init(&data->lock);

    data->miscdev.minor = MISC_DYNAMIC_MINOR;
    data->miscdev.name = "hcsr04";
    data->miscdev.fops = &hcsr04_fops;
    data->miscdev.parent = &pdev->dev;

    ret = misc_register(&data->miscdev);
    if (ret) return ret;

    platform_set_drvdata(pdev, data);
    return 0;
}

/* CRITICAL: Must be 'int' and return 0 for Kernel 6.9.8 */
static int hcsr04_remove(struct platform_device *pdev) {
    struct hcsr04_data *data = platform_get_drvdata(pdev);
    if (data) {
        misc_deregister(&data->miscdev);
    }
    return 0;
}

static const struct of_device_id hcsr04_dt_ids[] = {
    { .compatible = "hc-sr04" },
    { }
};
MODULE_DEVICE_TABLE(of, hcsr04_dt_ids);

static struct platform_driver hcsr04_driver = {
    .driver = {
        .name = "hc-sr04-driver",
        .of_match_table = hcsr04_dt_ids,
    },
    .probe = hcsr04_probe,
    .remove = hcsr04_remove,
};

module_platform_driver(hcsr04_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arno");
MODULE_DESCRIPTION("HC-SR04 Ultrasonic Distance Sensor Driver");