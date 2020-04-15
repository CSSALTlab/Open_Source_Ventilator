## Thanks for volunteering in collaborating with this project.

There is a playlist with some videos about this code:

[https://www.youtube.com/watch?v=gKr-EH8vHxM&list=PLtBsFN3o4c54h5fr7lDnGuZhvKIA5Fhd3](https://www.youtube.com/watch?v=gKr-EH8vHxM&list=PLtBsFN3o4c54h5fr7lDnGuZhvKIA5Fhd3)

I did not have a chance yet to document it as we are very very busy these days.

Following are things in my TODO list (other than adding features):

*  :white_check_mark:[DONE] Prevent the “params" array structure to use RAM. There is a huge limitation in this processor where even when you define a “const whatever” it allocates program space as well as a  DUPLICATION of it in RAM. I can give you more background about it  in case you decide to start wit that.

* Hack the Bootloader to pass the content of the watchdog register information. Actually the boot loader makes this register available in the processor R2. But the boot loader resets the flag that we are interesting in before calling out App. Again, I can give you more details and there are already boot loaders with this hack.

* Non initialized variable in RAM: we would like to have some variable stored in RAM that would survive resets. However, just by following the suggested procedures,  it does not work. My guess is that the linker scripts were changed in the toolchain and need to be hacked.

* Remove non used code. For example, I just changed LiquidCrystal library to be real-time and it is working great. We need to remove all functions that we do not need.

* Optimize RAM as much as we can: check all bool’s and int’s and replace by int8_t declarations. this is to save RAM.

* separate the config for each board in a dedicated header file (board_xxxx.h). config.h will just include a single board_xxxx.h file.

* get rid of most (if not all) floating point computations.

* better abstract pressure sensor options (NXP 7002 and BMP280)

* create a separated "discover" program to help guys integrating stepper motor with their mechanism: allow operator to move motor using buttons to extract parameters to be read and later written in the real controller code.

We may have more features coming as well.

At this point, while we do not have PCB's available for everyone, you would need a Arduino Nano to test your code. Even better if you have the same LCD we are using.

Regards,
Marcelo
