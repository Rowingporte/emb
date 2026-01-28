#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

struct hcsr04_data {
    struct gpio_desc *trig_gpio;
    struct gpio_desc *echo_gpio;
    int major;
};

/* Variable globale pour simplifier l'accès dans read/open */
static struct hcsr04_data *global_data = NULL;

static int hcsr04_open(struct inode *inode, struct file *file) {
    /* On lie le pointeur global au fichier pour le read */
    file->private_data = global_data;
    return 0;
}

static ssize_t hcsr04_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct hcsr04_data *data = file->private_data;
    ktime_t start, end;
    s64 delta_ns;
    char response[24];
    int len;

    if (!data) return -ENODEV;
    if (*ppos > 0) return 0;

    gpiod_set_value(data->trig_gpio, 1);
    udelay(10);
    gpiod_set_value(data->trig_gpio, 0);

    while (gpiod_get_value(data->echo_gpio) == 0);
    start = ktime_get();
    
    while (gpiod_get_value(data->echo_gpio) == 1);
    end = ktime_get();

    delta_ns = ktime_to_ns(ktime_sub(end, start));

    len = scnprintf(response, sizeof(response), "%lld\n", delta_ns);

    if (copy_to_user(buf, response, len)) return -EFAULT;
    
    *ppos += len;
    return len;
}

static const struct file_operations hcsr04_fops = {
    .owner = THIS_MODULE,
    .read = hcsr04_read,
    .open = hcsr04_open,
};

static int hcsr04_probe(struct platform_device *pdev) {
    struct device *dev = &pdev->dev;
    int major;

    global_data = devm_kzalloc(dev, sizeof(*global_data), GFP_KERNEL);
    if (!global_data) return -ENOMEM;

    global_data->trig_gpio = devm_gpiod_get(dev, "trig", GPIOD_OUT_LOW);
    global_data->echo_gpio = devm_gpiod_get(dev, "echo", GPIOD_IN);

    if (IS_ERR(global_data->trig_gpio) || IS_ERR(global_data->echo_gpio)) {
        dev_err(dev, "Erreur lors de la récupération des GPIOs\n");
        return -1;
    }

    major = register_chrdev(0, "hcsr04", &hcsr04_fops);
    if (major < 0) return major;
    
    global_data->major = major;
    dev_info(dev, "HCSR04 pret! Major: %d\n", major);
    
    platform_set_drvdata(pdev, global_data);
    return 0;
}

static int hcsr04_remove(struct platform_device *pdev) {
    struct hcsr04_data *data = platform_get_drvdata(pdev);
    if (data) {
        unregister_chrdev(data->major, "hcsr04");
    }
    return 0;
}

static const struct of_device_id hcsr04_dt_ids[] = {
    { .compatible = "hcsr04", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, hcsr04_dt_ids);

static struct platform_driver hcsr04_driver = {
    .probe = hcsr04_probe,
    .remove = hcsr04_remove,
    .driver = {
        .name = "hcsr04_sensor",
        .of_match_table = hcsr04_dt_ids,
    },
};

module_platform_driver(hcsr04_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arno - INSA");