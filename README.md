# game_controller
A linux device driver for a generic 3 axis 4 button game controller. Accompanied by a user space demo tool

This project was done by me some time ago to understand how to write a device driver/module that runs in the kernel space in Linux.

This readme is a summary of my understanding.





# Registering a driver 
The first step of any device module is to register it.
This is done by the macro - macro_init(function pointer)

The init The `module_init(function pointer)` macro causes the init function to be discarded and its memory freed once the init function finishes for built-in drivers. Here you can setup any resources for the driver(memory/counters etc)

# Unregistering a driver

This is what gets called when you unload a driver. This is registered with the macro `module_exit(function pointer)`
You must free any resourcse that you set up in module_init here.

# Building a driver

At the bare minimum, a drive must implement functions that will be called at time of loading and unloading.

With these two functions in place, we can go ahead and build our module.

Take a look at the [Makefile](/Makefile)
