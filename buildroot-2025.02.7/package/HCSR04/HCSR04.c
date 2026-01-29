#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/of.h>

struct hcsr04_data {
    struct gpio_desc *trig_gpio;
    struct gpio_desc *echo_gpio;
    int irq;
    int major;
    ktime_t start_time;
    s64 last_delta_ns;
    wait_queue_head_t wq;
    bool data_ready;
};

static struct hcsr04_data *global_data = NULL;

static irqreturn_t hcsr04_echo_isr(int irq, void *dev_id) {
    struct hcsr04_data *data = dev_id;
    int val = gpiod_get_value(data->echo_gpio);
    ktime_t now = ktime_get();

    if (val == 1) {
        // On ne démarre le chrono QUE si on n'est pas déjà en attente d'un écho
        if (data->data_ready == false) {
            data->start_time = now;
        }
    } else {
        // On ne calcule que si on a un début de mesure valide
        if (ktime_to_ns(data->start_time) > 0) {
            data->last_delta_ns = ktime_to_ns(ktime_sub(now, data->start_time));
            data->start_time = ktime_set(0, 0); // Reset
            data->data_ready = true;
            wake_up_interruptible(&data->wq);
        }
    }
    return IRQ_HANDLED;
}

static int hcsr04_open(struct inode *inode, struct file *file) {
    file->private_data = global_data;
    return 0;
}

static ssize_t hcsr04_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct hcsr04_data *data = file->private_data;
    char response[32];
    int len;

    if (!data) return -ENODEV;
    if (*ppos > 0) return 0;

    /* 1. Reset du drapeau et envoi du Trigger */
    data->data_ready = false;
    gpiod_set_value(data->trig_gpio, 1);
    udelay(10);
    gpiod_set_value(data->trig_gpio, 0);

    /* 2. Attente de l'interruption (Sommeil interruptible) */
    /* Timeout de 100ms au cas où le capteur est débranché */
    if (wait_event_interruptible_timeout(data->wq, data->data_ready, msecs_to_jiffies(100)) == 0) {
        return -ETIMEDOUT;
    }

    /* 3. Préparation de la réponse (Raw value en ns) */
    len = scnprintf(response, sizeof(response), "%lld\n", data->last_delta_ns);

    if (copy_to_user(buf, response, len))
        return -EFAULT;

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
    int ret;

    global_data = devm_kzalloc(dev, sizeof(*global_data), GFP_KERNEL);
    if (!global_data) return -ENOMEM;

    /* Initialisation de la file d'attente */
    init_waitqueue_head(&global_data->wq);

    /* Récupération des GPIOs */
    global_data->trig_gpio = devm_gpiod_get(dev, "trig", GPIOD_OUT_LOW);
    if (IS_ERR(global_data->trig_gpio)) return PTR_ERR(global_data->trig_gpio);

    global_data->echo_gpio = devm_gpiod_get(dev, "echo", GPIOD_IN);
    if (IS_ERR(global_data->echo_gpio)) return PTR_ERR(global_data->echo_gpio);

    /* Configuration de l'interruption */
    global_data->irq = gpiod_to_irq(global_data->echo_gpio);
    if (global_data->irq < 0) return global_data->irq;

    ret = devm_request_irq(dev, global_data->irq, hcsr04_echo_isr,
                           IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                           "hcsr04_echo_irq", global_data);
    if (ret) {
        dev_err(dev, "Impossible de demander l'IRQ %d\n", global_data->irq);
        return ret;
    }

    /* Enregistrement du char device */
    global_data->major = register_chrdev(0, "hcsr04", &hcsr04_fops);
    if (global_data->major < 0) return global_data->major;

    dev_info(dev, "HCSR04 Interrupt Driver chargé! Major: %d, IRQ: %d\n", 
             global_data->major, global_data->irq);
    
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