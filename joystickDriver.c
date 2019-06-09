#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>

#include <net/netlink.h>
#include <net/net_namespace.h>

MODULE_LICENSE("GPL");

struct urb* my_urb;
void* my_buffer;
static struct usb_interface* my_interface;
static const struct usb_device_id* my_id;
static int pen_probe(struct usb_interface*, const struct usb_device_id*);

struct controller
{
    char xaxis;
    char yaxis;
    char slider;
    char b1;
    char b2;
    char b3;
    char b4;
};

static struct usb_device_id pen_table[] = {

    { USB_DEVICE(0x0583, 0xb03b) }, {},
};

/* Protocol family, consistent in both kernel prog and user prog. */
#define MYPROTO NETLINK_USERSOCK
/* Multicast group, consistent in both kernel prog and user prog. */
#define MYGRP 31

static struct sock* nl_sk = NULL;

static void send_netlink_message(void)
{
    struct sk_buff* skb;
    struct nlmsghdr* nlh;
    char* msg = "Hello from kernel";
    int msg_size = strlen(msg) + 1;
    int res;

    pr_info("Creating skb.\n");
    skb = nlmsg_new(NLMSG_ALIGN(msg_size + 1), GFP_KERNEL);
    if (!skb)
    {
        pr_err("Allocation failure.\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 1, NLMSG_DONE, msg_size + 1, 0);
    strcpy(nlmsg_data(nlh), msg);
    pr_info("Sending skb.\n");
    res = nlmsg_multicast(nl_sk, skb, 0, MYGRP, GFP_KERNEL);
    if (res < 0)
        pr_info("nlmsg_multicast() error: %d\n", res);
    else
        pr_info("Success.\n");
}

void myHandler(struct urb* urb)
{
    char* buffer = (char*)urb->transfer_buffer;
    struct controller ontroller;

    ontroller.xaxis = buffer[0];
    ontroller.yaxis = buffer[1];
    ontroller.updownaxis = buffer[2];
    ontroller.b1 = buffer[3];
    ontroller.b2 = buffer[4];
    ontroller.b3 = buffer[5];
    ontroller.b4 = buffer[6];

    if (ontroller.b1 + ontroller.b2 + ontroller.b3 + ontroller.b4 > 0)
    {
        send_netlink_message();
    }
    usb_submit_urb(urb, GFP_ATOMIC);
}
static struct usb_device* device;

MODULE_DEVICE_TABLE(usb, pen_table);

static int generic_joystick_probe(struct usb_interface* interface, const struct usb_device_i$
{

    int pipe;
    unsigned int buffer_size;
    struct usb_host_interface* iface_desc;
    struct usb_endpoint_descriptor* endpoint;

    usb_complete_t handler = myHandler;

    my_interface = interface;
    my_id = id;

    printk(KERN_INFO "[*]joystick pluggedin VendorId %d ProductId %d \n", id->idVend, idProduct);

    iface_desc = interface->cur_altsetting;
    endpoint = &iface_desc->endpoint[0].desc;

    device = interface_to_usbdev(interface);
    my_urb = usb_alloc_urb(0, GFP_ATOMIC);

    pipe = usb_rcvintpipe(device, endpoint->bEndpointAddress);
    buffer_size = endpoint->wMaxPacketSize;
    my_buffer = kmalloc(buffer_size, GFP_ATOMIC);
        usb_fill_int_urb(my_urb, device, pipe, my_buffer, buffer_size, handler,$
        usb_submit_urb(my_urb, GFP_ATOMIC);
        return 0;

}


static void generic_joystick_disconnect(struct usb_interface* interface)
{
        printk(KERN_INFO "[*] joystick removed \n");


}


static struct usb_driver pen_driver =
{
        .name = "GenericJoystick",
        .id_table = pen_table,
        .probe = generic_joystick_probe,
        .disconnect = generic_joystick_disconnect,
};

static int __init pen_init(void)
{
        int ret;
        nl_sk = netlink_kernel_create(&init_net, MYPROTO, NULL);
        if (!nl_sk)

        {
            pr_err("Error creating socket.\n");
            return -10;
        }

        ret = -1;
        printk(KERN_INFO "saimadhav registering driver for joystick with kernel");
        ret = usb_register(&pen_driver);
        printk(KERN_INFO "saimadhav registration is complete");

        return ret;
}
static void __exit pen_exit(void)
{
        printk(KERN_INFO "saimadhav unregeisted log file\n");
        {
            printk(KERN_INFO "saimadhav unregeisted log file\n");
            usb_deregister(&pen_driver);
            netlink_kernel_release(nl_sk);

            printk(KERN_INFO "saimadhav deregistration comlete\n");
        }

        module_init(pen_init);
        module_exit(pen_exit);
