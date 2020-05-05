//TODO: untangle Gibby's code and put the core here
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <Python.h>
#include "hardware_interface.h"

int main()
{

  initialize_hardware();

  //printf("a");
  while(true)
  {
    uart_print("hello world!\n");
    //run core software
    // get data from sensors
    // act on data through lcd / motor
  }
}
