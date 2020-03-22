#include "vent.h"
#include "hal.h"

void setup() {
  halInit();
  ventSetup();
}

void loop() {
  ventLoop();  
}
