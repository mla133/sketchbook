/*************************************************** 
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED Matrix backpacks 
  ----> http://www.adafruit.com/products/872
  ----> http://www.adafruit.com/products/871
  ----> http://www.adafruit.com/products/870

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
****************************************************/
 
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

const byte numChars = 32;
char receivedChars[numChars]; // an array to store incoming data
boolean newData = false;

void setup() {
  Serial.begin(19200);
  Serial.println("8x8 LED Matrix Test");
  matrix.begin(0x70);  // pass in the address
  matrix.setRotation(2);      // Orients text so bottom is facing input pins
  matrix.setTextSize(1);      // Only TextSize that fits 8x8 matrix standalone
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix.clear();
}

void loop() {
      recvWithEndMarker();
      if(newData) {
        showNewData(100);  
      }
      else {
        scan_display(50);
      }     
}

void recvWithEndMarker(){
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while(Serial.available() > 0 && newData == false) {
      rc = Serial.read();
      if (rc != endMarker){
        receivedChars[ndx] = rc;
        ndx++;
//        if(ndx >= numChars){
//          ndx = numChars - 1;
//        }
      }
      else{
        receivedChars[ndx] = '\0'; //terminate string
        ndx = 0;
        newData = true;
      }
   }
}

void showNewData(int8_t speed) {
  //Serial.print("Received: ");
  //Serial.println(receivedChars);
  
  for (int8_t x=0; x>=-72; x--) {
    matrix.clear();
    matrix.setCursor(x,0);
    matrix.print(receivedChars);
    matrix.writeDisplay();
    delay(speed);
  }
  newData = false;
}

void scan_display(int8_t speed) {
  // Scan line display showing idleness
  for(int8_t y=0; y<8; y++){
    for(int8_t x=0; x<8; x++){
      matrix.clear();
      matrix.drawLine(x, 1, x, 6, LED_ON);  
      matrix.writeDisplay();
      delay(speed);
    }
  } 
}
