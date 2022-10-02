#include <Arduino.h>

#include "A"
#include "WiFiMulti.h"

#define I2S_DOUT      6
#define I2S_BCLK      7
#define I2S_LRC       8

 audio;


void setup() {
  // write your initialization code here

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

}

void loop() {
// write your code here
}