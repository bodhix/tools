/**
 * test notify chain
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/notifier.h>

#include <linux/netdevice.h>

#define VRESION "1.0.0"
#define LICENSE "GPL"
#define AUTHOR  "cbzhang"

static int notifier_func(struct notifier_block *nb, unsigned long event, void *data)
{
    struct net_device *dev = netdev_notifier_info_to_dev(data);
    printk(KERN_INFO "[notifier_chain.ko] get event %lu of dev %s[%d]", event, dev->name, dev->ifindex);
    return 0;
}

static struct notifier_block notifier_z = {
    .notifier_call = notifier_func,
    .next = NULL,
    .priority = 0
};

// param void must be added, otherwise compile would failed
static int __init init_notifier(void)
{
    int ret;
    printk(KERN_INFO "[notifier_chain.ko] init notifier\n");

    printk(KERN_INFO "[notifier_chain.ko] registering netdev notifier...\n");
    ret = register_netdevice_notifier(&notifier_z);
    if (ret)
    {
        printk(KERN_ERR "[notifier_chain.ko] registering failed: %d\n", ret);
    }
    else
    {
        printk(KERN_INFO "[notifier_chain.ko] registering succeed: %d\n", ret);
    }
    return ret;
}

// param void must be added, otherwise compile would failed
static void __exit exit_notifier(void)
{
    printk(KERN_INFO "[notifier_chain.ko] exit notifier\n");

    printk(KERN_INFO "[notifier_chain.ko] unregistering netdev notifier...\n");
    unregister_netdevice_notifier(&notifier_z);
}

module_init(init_notifier)
module_exit(exit_notifier)
MODULE_AUTHOR(AUTHOR);
MODULE_LICENSE(LICENSE);
MODULE_VERSION(VRESION);
