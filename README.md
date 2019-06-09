# game_controller
A linux device driver for a generic 3 axis 4 button game controller. Accompanied by a user space demo tool

This project was done by me some time ago to understand how to write a device driver/module that runs in the kernel space in Linux.

This readme is a summary of my understanding.

# The bare minimum driver skeleton



## Function to handle driver registration
The first step of any device module is to register it.
This is done by the macro - macro_init(function pointer)

The init The `module_init(function pointer)` macro causes the init function to be discarded and its memory freed once the init function finishes for built-in drivers. Here you can setup any resources for the driver(memory/counters etc)

## Function to handle driver unregistration

This is what gets called when you unload a driver. This is registered with the macro `module_exit(function pointer)`
You must free any resourcse that you set up in module_init here.

## Building a driver

At the bare minimum, a drive must implement functions that will be called at time of loading and unloading.

With these two functions in place, we can go ahead and build our module.

Take a look at the [Makefile](/Makefile)

## Loading a driver

If successfully built, we can go ahead and load the driver. 
```
sudo insmod <name of kernel object file (.ko extension of previous step)>
```

Any PRINTK\alerts messages can be seen by using `dmesg -w`. This is because you cannot use `printf` family of functions while the code is running in the kernel

## Unloading a driver

One can remove a loaded driver by 
```
sudo rmmod <namke of driver>
```

This covers the bare minimums of driver registration/unregistration. Next we will look at how to react when the device of interest is plugged into system. Here were are dealing with a USB game controller



# Handling device plugging and ejection



## The USB architecture
Now is a good time to read http://lmu.web.psi.ch/docu/manuals/software_manuals/linux_sl/usb_linux_programming_guide.pdf which provides a good idea about the USB architecture on Linux(USB is same everywhere!)


## The probe

The function you regsiter for the probe gets called when the device gets plugged in. Here you can create device specific data structures. Difference between probe and init is that probe is device specific(if you attach two game controllers, probe gets called twice. Init gets called only once, when the *driver* is loaded)


## The disconnect
This is called when you eject the device from your system. Free up any data structures that were associated with the device here.


# Interacting with the device

This is a really broad topic. Depending on nature of device, its interfaces(BULK/INTERRUPT/CONTROL/ISOCHRNOUS) you can do various things(like read/write). 
This is accomplished using the data structure
USB request block
```
struct urb
{
// (IN) device and pipe specify the endpoint queue
      struct usb_device *dev;         // pointer to associated USB device
      unsigned int pipe;              // endpoint information

      unsigned int transfer_flags;    // URB_ISO_ASAP, URB_SHORT_NOT_OK, etc.

// (IN) all urbs need completion routines
      void *context;                  // context for completion routine
      usb_complete_t complete;        // pointer to completion routine

// (OUT) status after each completion
      int status;                     // returned status

// (IN) buffer used for data transfers
      void *transfer_buffer;          // associated data buffer
      u32 transfer_buffer_length;     // data buffer length
      int number_of_packets;          // size of iso_frame_desc

// (OUT) sometimes only part of CTRL/BULK/INTR transfer_buffer is used
      u32 actual_length;              // actual data buffer length

// (IN) setup stage for CTRL (pass a struct usb_ctrlrequest)
      unsigned char *setup_packet;    // setup packet (control only)

// Only for PERIODIC transfers (ISO, INTERRUPT)
  // (IN/OUT) start_frame is set unless URB_ISO_ASAP isn't set
      int start_frame;                // start frame
      int interval;                   // polling interval

  // ISO only: packets are only "best effort"; each can have errors
      int error_count;                // number of errors
      struct usb_iso_packet_descriptor iso_frame_desc[0];
};
```

Read more at https://www.kernel.org/doc/html/latest/driver-api/usb/URB.html


Once the URB request is submitted, and once the device responds, there is call made to the URB completion handler. In our case, we recieve the current inputs of the game controller


# Deciphering the meaning of USB signals

At the device level, every action(button press or axis movement) is a pattern of bits. Whenever you peform this action, the USB device via the USB host controller places a bit pattern into the USB bus. It ultimately gets passed to the concerned device driver.

Making sense of this bit pattern is trivial if you have access to the hardware manual. However in my case, I did not.

In this scenario, understanding the meaning of each action(and the corressponding bit pattern) reduces to reverse engineering. You can use USB sniffing tools like USBPCap etc for this purpose. After much permutation and combination, I deciphered the following information about the device I had 

![Bit pattern of game controller buffer](/bitpattern.jpeg)

Each time a response is with us, it means the user did some action - moved the joystick or pressed a button. We need to take some action. But the kernel(device driver) is not the best place to take such action. The role of the device driver is just to talk to the hardware and the user space and nothing more.

With this philosophy, we just pass on all user actions(button press/axis movement etc) to the user space

# Communicating to user space

There are many ways of communicating to the user space from the kernel. Some ways are using `procfs`, `sysfs`, `ioctl` or `debugfs`.

They all have their own pros and cons.

I decided to go with `netlink`

## netlink

In short, netlink is a socket based mechanism for communication between kernel and userspace. Conceptually, it exposes interface similar to TCP/UDP sockets. So development of senders/recievers is a breeze if you are familiar with socket programming

You can take a look at `send_netlink_message` function in kernel driver and the sockets code in the user application for its simplicity

