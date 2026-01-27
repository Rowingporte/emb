#include <linux/module.h>
#include <linux/kernel.h>

static int __init hello_init(void) {
    printk(KERN_INFO "Hello: Kernel world initialized\n");
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "Hello: Cleaning up module\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");