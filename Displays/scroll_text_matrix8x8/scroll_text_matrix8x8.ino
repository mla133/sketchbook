#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

void setup() {
  matrix.begin(0x70);  // pass in the address
  matrix.setRotation(2);
  matrix.setTextSize(1);
  matrix.setTextWrap(true);
  matrix.setTextColor(LED_ON); 
}

void loop() {
      int8_t i = 0;
      matrix.clear();
      matrix.setCursor(1,0);
      matrix.print(itoa(i));
      delay(250);

//  for (int8_t x=0; x>=-1; x--) {
//    matrix.clear();
//    matrix.setCursor(x,0);
//    matrix.print("05-14-20");
//    matrix.writeDisplay();
//    delay(150);
//  }
  
//  delay(500);
}
