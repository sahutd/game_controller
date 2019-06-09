#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

MODULE_LICENSE("GPL");

struct urb *my_urb;
void *my_buffer;
static struct usb_interface *my_interface;
static const struct usb_device_id *my_id;
static int joystick_probe(struct usb_interface *, const struct usb_device_id *);
static void joystick_disconnect(struct usb_interface *interface);

struct controller {
  int xaxis;
  int yaxis;
  unsigned int updownaxis;
  int b1;
  int b2;
  int b3;
  int b4;
};


static struct usb_device_id joystick_table[] = {

    {USB_DEVICE(0x0583, 0xb03b)}, {},
};


static struct usb_driver pen_driver = {
    .name = "Generic Joystick",
    .id_table = joystick_table,
    .probe = joystick_probe,
    .disconnect = joystick_disconnect,
};


/* Protocol family, consistent in both kernel prog and user prog. */
#define MYPROTO NETLINK_USERSOCK
/* Multicast group, consistent in both kernel prog and user prog. */
#define MYGRP 31

static struct sock *nl_sk = NULL;

static void send_netlink_message(struct controller* controller) {
  struct sk_buff *skb;
  struct nlmsghdr *nlh;
  char msg[50];
  int msg_size;
  int res;
  snprintf( msg, 50, "%d%d%d%d,%u,%u,%u", controller->b1, controller->b2, controller->b3, controller->b4, controller->xaxis, controller->yaxis,controller->updownaxis);
  msg_size = strlen(msg) + 1;



  skb = nlmsg_new(NLMSG_ALIGN(msg_size + 1), GFP_KERNEL);
  if (!skb) {
    pr_err("Allocation failure.\n");
    return;
  }

  nlh = nlmsg_put(skb, 0, 1, NLMSG_DONE, msg_size + 1, 0);
  strcpy(nlmsg_data(nlh), msg);

 
  res = nlmsg_multicast(nl_sk, skb, 0, MYGRP, GFP_KERNEL);

}

void myHandler(struct urb *urb) {

  uint8_t *buffer = (uint8_t *)urb->transfer_buffer;
  struct controller controller;

  controller.xaxis = buffer[0];
  controller.yaxis = buffer[1];
  controller.updownaxis = buffer[2];  
  controller.b1 = buffer[3] & 1 << 0 ? 1 : 0;
  controller.b2 = buffer[3] & 1 << 1 ? 1 : 0;
  controller.b3 = buffer[3] & 1 << 2 ? 1 : 0;
  controller.b4 = buffer[3] & 1 << 3 ? 1 : 0;

  send_netlink_message(&controller);
  usb_submit_urb(urb, GFP_ATOMIC);
}

static struct usb_device *device;

MODULE_DEVICE_TABLE(usb, joystick_table);

static int joystick_probe(struct usb_interface *interface,
                     const struct usb_device_id *id) {

  int pipe;
  unsigned int buffer_size;
  struct usb_host_interface *iface_desc;
  struct usb_endpoint_descriptor *endpoint;

  usb_complete_t handler = myHandler;
  my_interface = interface;
  my_id = id;

  printk(KERN_INFO "[*] Generic joystick pluggedin %d %d \n", id->idVendor,
         id->idProduct);

  iface_desc = interface->cur_altsetting;


  endpoint = &iface_desc->endpoint[0].desc;

  device = interface_to_usbdev(interface);
  my_urb = usb_alloc_urb(0, GFP_ATOMIC);

  pipe = usb_rcvintpipe(device, endpoint->bEndpointAddress);
  buffer_size = endpoint->wMaxPacketSize;
  my_buffer = kmalloc(buffer_size, GFP_ATOMIC);
  usb_fill_int_urb(my_urb, device, pipe, my_buffer, buffer_size, handler, NULL,
                   endpoint->bInterval);
  usb_submit_urb(my_urb, GFP_ATOMIC);
  return 0;
}

static void joystick_disconnect(struct usb_interface *interface) {
  printk(KERN_INFO "[*] Generic joystick removed \n");
}



static int __init pen_init(void) {
  int ret;
  nl_sk = netlink_kernel_create(&init_net, MYPROTO, NULL);
  if (!nl_sk) {
    pr_err("Error creating socket.\n");
    return -10;
  }

  ret = -1;
  printk(KERN_INFO "[*] Registering driver for joystick with kernel");
  ret = usb_register(&pen_driver);
  printk(KERN_INFO "[*] Registration is complete");

  return ret;
}

static void __exit pen_exit(void) {
  printk(KERN_INFO "[*] Unregistering joystick\n");
  usb_deregister(&pen_driver);
  netlink_kernel_release(nl_sk);
  printk(KERN_INFO "[*] Unregistering complete\n");

}

module_init(pen_init);
module_exit(pen_exit);
