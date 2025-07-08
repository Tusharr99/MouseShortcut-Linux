#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/input.h>
#include <net/sock.h>
#include <net/netlink.h>

#define NETLINK_USER 29
#define MAX_PAYLOAD 64

static struct sock *nl_sk = NULL;
static struct input_handler mouse_handler;
static struct input_handle *mouse_handle;
static int user_pid = 0; // Store user-space PID
static int debug = 0; // Debug flag

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable debug logging (0=off, 1=on)");

#define DBG(fmt, ...) do { if (debug) pr_debug("mouse_shortcut: " fmt, ##__VA_ARGS__); } while (0)

// Netlink receive callback to register user-space PID
static void nl_receive(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    if (NLMSG_OK(nlh, skb->len) && strcmp(NLMSG_DATA(nlh), "REGISTER") == 0) {
        user_pid = nlh->nlmsg_pid;
        pr_info("Registered user-space PID: %d\n", user_pid);
    }
}

static void mouse_event(struct input_handle *handle, unsigned int type,
                        unsigned int code, int value)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    char msg[MAX_PAYLOAD];

    // Log all events
    DBG("Received event: type=%u, code=%u, value=%d from device %s\n",
        type, code, value, handle->dev->name);

    // Process only EV_KEY events for buttons 275 and 276
    if (type != EV_KEY || (code != 275 && code != 276))
        return;

    snprintf(msg, MAX_PAYLOAD, "Button %u %s", code,
             value ? "pressed" : "released");
    DBG("Preparing to send: %s\n", msg);

    // Allocate socket buffer
    skb = nlmsg_new(MAX_PAYLOAD, GFP_KERNEL);
    if (!skb) {
        pr_err("Failed to allocate skb\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 0, NLMSG_DEFAULT_SIZE, MAX_PAYLOAD, 0);
    if (!nlh) {
        pr_err("Failed to put nlmsg\n");
        kfree_skb(skb);
        return;
    }

    strcpy(nlmsg_data(nlh), msg);

    // Send to user-space if PID is registered
    if (user_pid) {
        DBG("Sending to user-space (PID=%d): %s\n", user_pid, msg);
        if (netlink_unicast(nl_sk, skb, user_pid, MSG_DONTWAIT) < 0) {
            pr_err("Failed to send netlink message to PID %d\n", user_pid);
            kfree_skb(skb);
        } else {
            DBG("Netlink message sent successfully\n");
        }
    } else {
        DBG("No user-space PID registered, dropping message\n");
        kfree_skb(skb);
    }
}

static int mouse_connect(struct input_handler *handler, struct input_dev *dev,
                         const struct input_device_id *id)
{
    int ret;

    DBG("Attempting to connect to device: %s (vendor=0x%04x, product=0x%04x, bustype=0x%04x)\n",
        dev->name, dev->id.vendor, dev->id.product, dev->id.bustype);

    // Only connect to VirtualBox mouse integration
    if (dev->id.vendor != 0x80ee || dev->id.product != 0xcafe) {
        DBG("Skipping device: %s (not VirtualBox mouse integration)\n", dev->name);
        return -ENODEV;
    }

    mouse_handle = kzalloc(sizeof(*mouse_handle), GFP_KERNEL);
    if (!mouse_handle) {
        pr_err("Failed to allocate mouse_handle\n");
        return -ENOMEM;
    }

    mouse_handle->dev = dev;
    mouse_handle->handler = handler;
    mouse_handle->name = "mouse_shortcut";
    mouse_handle->private = NULL;

    ret = input_register_handle(mouse_handle);
    if (ret) {
        pr_err("Failed to register input handle\n");
        kfree(mouse_handle);
        return ret;
    }

    // Grab the device to prevent other handlers from consuming events
    ret = input_open_device(mouse_handle);
    if (ret) {
        pr_err("Failed to open input device\n");
        input_unregister_handle(mouse_handle);
        kfree(mouse_handle);
        return ret;
    }

    pr_info("Connected to mouse device: %s (vendor=0x%04x, product=0x%04x, bustype=0x%04x)\n",
            dev->name, dev->id.vendor, dev->id.product, dev->id.bustype);
    return 0;
}

static void mouse_disconnect(struct input_handle *handle)
{
    pr_info("Disconnected from mouse device: %s\n", handle->dev->name);
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
}

static const struct input_device_id mouse_ids[] = {
    {
        .flags = INPUT_DEVICE_ID_MATCH_VENDOR | INPUT_DEVICE_ID_MATCH_PRODUCT | INPUT_DEVICE_ID_MATCH_EVBIT,
        .vendor = 0x80ee,
        .product = 0xcafe,
        .evbit = { BIT_MASK(EV_KEY) },
    },
    {}, // Terminate the list
};

static struct input_handler mouse_handler = {
    .event = mouse_event,
    .connect = mouse_connect,
    .disconnect = mouse_disconnect,
    .name = "mouse_shortcut",
    .id_table = mouse_ids,
};

static int __init mouse_shortcut_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = nl_receive,
    };
    int ret;

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        pr_err("Failed to create netlink socket for protocol %d\n", NETLINK_USER);
        return -ENOMEM;
    }

    pr_info("Netlink socket created for protocol %d\n", NETLINK_USER);

    ret = input_register_handler(&mouse_handler);
    if (ret) {
        pr_err("Failed to register input handler\n");
        netlink_kernel_release(nl_sk);
        return ret;
    }

    pr_info("Mouse shortcut module loaded\n");
    return 0;
}

static void __exit mouse_shortcut_exit(void)
{
    input_unregister_handler(&mouse_handler);
    if (nl_sk)
        netlink_kernel_release(nl_sk);
    pr_info("Mouse shortcut module unloaded\n");
}

module_init(mouse_shortcut_init);
module_exit(mouse_shortcut_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Mouse shortcut kernel module using Netlink");