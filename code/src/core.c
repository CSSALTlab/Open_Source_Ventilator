//TODO: untangle Gibby's code and put the core here
#include <stdbool.h>
#include <stdint.h>
#include "hardware_interface.h"

int main()
{

  initialize_hardware();

  // Hardware interface checks
  uart_print("hello world!\n");
  lcd_print(0, 0, "abcdefghijklmnopqrstu");
  lcd_print(1, 1,  "bcdefghijklmnopqrstu");
  lcd_print(2, 2,   "cdefghijklmnopqrstu");
  lcd_print(3, 3,    "defghijklmnopqrstu");

  sound_alarm();
  delay_ms(1000);
  silence_alarm();

  while(true)
  {
    //run core software
    // get data from sensors
    // act on data through lcd / motor / alarm
  }
}
