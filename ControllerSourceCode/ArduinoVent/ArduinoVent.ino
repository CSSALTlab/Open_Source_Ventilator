#include "vent.h"
#include "hal.h"

//#define GET_RESET_VAL // disable as this code is not working
#ifdef GET_RESET_VAL
uint8_t resetFlags __attribute__ ((section(".noinit")));
void resetFlagsInit(void) __attribute__ ((naked)) __attribute__ ((section (".init0")));
void resetFlagsInit(void)
{
  // save the reset flags passed from the bootloader
  __asm__ __volatile__ ("mov %0, r2\n" : "=r" (resetFlags) :);
}
#else
void resetFlagsInit() { /* dummy */ }
uint8_t resetFlags = 0;
#endif

void setup() {
  resetFlagsInit();
  halInit(resetFlags);
  ventSetup();
}

void loop() {
  ventLoop();  
}
