#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 14
#define EEPROM_D0 5
#define EEPROM_D7 12

void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address>>8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte readEEPROM(int address) {
  setAddress(address, /*outputEnable */ true);
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin =-1) {
    data = (data << 1) + digitalRead(pin);    
  }
  return data;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  Serial.begin(57600);

  Serial.println(readEEPROM(0));
  Serial.println(readEEPROM(1));
  Serial.println(readEEPROM(2));
  Serial.println(readEEPROM(3));
  
  

  /*
  for (int base = 0; base <= 255; base += 16) {
    byte data[16];
    for (int offset = 0; offset <=15; offset +=1) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[80];
    sprintf(buf, "%03x: %02 %02x %02x %02x %02x %02x %02x %02x | %02 %02x %02x %02x %02x %02x %02x %02x", 
      base, data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],
      data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
      
      Serial.println(buf);
   }
   */

  
}

void loop() {
  // put your main code here, to run repeatedly:

}
