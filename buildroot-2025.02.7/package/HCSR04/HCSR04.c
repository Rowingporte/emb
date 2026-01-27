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

struct hcsr04_data {
    struct gpio_desc *trig;
    struct gpio_desc *echo;
};

// Instance globale pour simplifier l'accès dans le read
static struct hcsr04_data *sensor_data;

static s64 get_raw_duration_us(struct hcsr04_data *data) {
    ktime_t start, end;
    int timeout;

    // Trigger pulse
    gpiod_set_value(data->trig, 1);
    udelay(10);
    gpiod_set_value(data->trig, 0);

    // Attente du signal HIGH (avec timeout)
    timeout = 1000;
    while (gpiod_get_value(data->echo) == 0 && timeout--) udelay(1);
    start = ktime_get();

    // Attente du signal LOW (avec timeout)
    timeout = 30000; // ~5 mètres max
    while (gpiod_get_value(data->echo) == 1 && timeout--) udelay(1);
    end = ktime_get();

    return ktime_to_us(ktime_sub(end, start));
}

static ssize_t hcsr04_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char buffer[16];
    int len;
    s64 duration;

    if (*ppos > 0) return 0;

    duration = get_raw_duration_us(sensor_data);
    len = sprintf(buffer, "%lld\n", duration);

    if (copy_to_user(buf, buffer, len)) return -EFAULT;
    
    *ppos = len;
    return len;
}

// On enlève le 'const' pour pouvoir l'utiliser dynamiquement ou on initialise tout ici
static const struct file_operations hcsr04_fops = {
    .owner = THIS_MODULE,
    .read = hcsr04_read,
    .open = simple_open,
};

static struct miscdevice hcsr04_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hcsr04",
    .fops = &hcsr04_fops,
};

static int hcsr04_probe(struct platform_device *pdev) {
    int ret;

    sensor_data = devm_kzalloc(&pdev->dev, sizeof(*sensor_data), GFP_KERNEL);
    if (!sensor_data) return -ENOMEM;

    sensor_data->trig = devm_gpiod_get(&pdev->dev, "trig", GPIOD_OUT_LOW);
    sensor_data->echo = devm_gpiod_get(&pdev->dev, "echo", GPIOD_IN);

    if (IS_ERR(sensor_data->trig) || IS_ERR(sensor_data->echo)) {
        dev_err(&pdev->dev, "Erreur GPIO\n");
        return -1;
    }

    ret = misc_register(&hcsr04_misc);
    if (ret) return ret;

    dev_info(&pdev->dev, "HCSR04: Driver charge, device /dev/hcsr04 cree\n");
    return 0;
}

static int hcsr04_remove(struct platform_device *pdev) {
    misc_deregister(&hcsr04_misc);
    return 0;
}

static const struct of_device_id hcsr04_dt_ids[] = {
    { .compatible = "capteur", },
    { }
};
MODULE_DEVICE_TABLE(of, hcsr04_dt_ids);

static struct platform_driver hcsr04_driver = {
    .driver = {
        .name = "hcsr04_driver",
        .of_match_table = hcsr04_dt_ids,
    },
    .probe = hcsr04_probe,
    .remove = hcsr04_remove,
};

module_platform_driver(hcsr04_driver);
MODULE_LICENSE("GPL");