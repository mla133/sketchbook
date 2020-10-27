#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

//definition of ALU_commands   (4 bit ALU, 4 control lines)                    
#define ALU_PASS_A  0x00                                                 
#define ALU_PASS_B  0x01                                                                                                                         
#define ALU_ROL_A   0x02                                                 
#define ALU_ROR_A   0x03                                                 
#define ALU_LSL_A   0x04                                                 
#define ALU_LSR_A   0x05                                                                                                                         
#define ALU_ADD     0x06                                                 
#define ALU_SUB     0x07                                                 
#define ALU_ADC     0x08                                                 
#define ALU_SBC     0x09                                                                                                                         
#define ALU_AND     0x0a                                                 
#define ALU_OR      0x0b                                                 
#define ALU_XOR     0x0c                                                                                                                        
#define ALU_CONST_0 0x0d                                                 
#define ALU_CONST_1 0x0e                                                 
#define ALU_CONST_F 0x0f 

// 4-bit hex decoder for common anode 7-segment display
//byte data[] = { 0x81, 0xcf, 0x92, 0x86, 0xcc, 0xa4, 0xa0, 0x8f, 0x80, 0x84, 0x88, 0xe0, 0xb1, 0xc2, 0xb0, 0xb8 };

// 4-bit hex decoder for common cathode 7-segment display
//byte data[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47 };

// 4-bit numerical sequence for testing memory outside of arduino environment
//  byte data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };


// Output the address bits and outputEnable signal using shift registers.
void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}


// Read a byte from the EEPROM at the specified address.
byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, INPUT);
  }
  setAddress(address, /*outputEnable*/ true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}



// Write a byte to the EEPROM at the specified address. 
void writeEEPROM(int address, byte data) {
  setAddress(address, /*outputEnable*/ false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

// Read the contents of the EEPROM and print them to the serial monitor.
void printContents() {
  for (int base = 0; base < 8192; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }
    char buf[80];
    sprintf(buf, "%05X:  %02X %02X %02X %02X %02X %02X %02X %02X   %02X %02X %02X %02X %02X %02X %02X %02X",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}

void eraseEEPROM() {
   // Erase entire EEPROM
  Serial.print("Erasing EEPROM");
  for (int address = 0; address <= 8192; address += 1) {
    writeEEPROM(address, 0xff);

    if (address % 64 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" done");
}

void programEEPROM() {
  // Program data bytes
  Serial.print("Programming EEPROM");
  for (int address = 0; address < 8192; address += 1) {
    writeEEPROM(address, do_rom(address));

    if (address % 64 == 0) {
      Serial.print(".");
    }
  }
  Serial.println(" done");  
}

char do_rom(long addr)                                                   
{                                                                        
  char out;                                                              
  char cmd,a,b;                                                          
  char cin;       //carry input                                          
  char cout,zout; //flags                                                
                                                                         
 //default: clear flags, clear result                                    
  out=0;                                                                 
  cin=0; cout=0; zout=0;                                                 
                                                                         
 //EPROM:                                                                
 //A0..3=A, A4..7=B,   A8..11=ALU_command, A12=Carry_in                  
 //Q0..3=Q, Q4=Z_Flag, Q7 =C_Flag                                        
  a  = addr     & 0x0f;                                                  
  b  =(addr>>4) & 0x0f;                                                  
  cmd=(addr>>8) & 0x0f;                                                  
                                                                         
  if((long)addr & 0x1000) cin=-1; //carry input                          
                                                                         
  switch(cmd)                                                            
  {                                                                      
    case ALU_PASS_A: out=a; break;                                       
    case ALU_PASS_B: out=b; break;                                       
                                                                         
    case ALU_ROL_A:  if(cin) out=0x01; //the rest like LSL               
    case ALU_LSL_A:  out|=(a<<1)&0x0e; if((char)a&0x08){cout=-1;} break; 
                                                                         
    case ALU_ROR_A:  if(cin) out=0x08; //the rest like LSR               
    case ALU_LSR_A:  out|=(a>>1)&0x07; if((char)a&0x01){cout=-1;} break; 
                                                                         
    case ALU_AND:    out=a&b; break;                                     
    case ALU_OR:     out=a|b; break;                                     
    case ALU_XOR:    out=a^b; break;                                     
                                                                         
    case ALU_ADC:    if(cin) out=1;                                      
    case ALU_ADD:    out+=(a&0xf)+(b&0xf);                               
                     if(out>0x0f){cout=-1;} break;                       
                                                                         
    case ALU_SBC:    if(cin==0) out=-1; //subtract 1, borrow active.     
    case ALU_SUB:    out+=(a&0xf)-(b&0xf);                               
                     if(out>=0){cout=-1;/*no borrow.*/} break;           
                                                                         
    case ALU_CONST_0: out=0x00; break;                                   
    case ALU_CONST_1: out=0x01; break;                                   
    case ALU_CONST_F: out=0x0f; break;                                   
  }                                                                      
                                                                         
  out&=0x0f; //limit result to 4 Bit, to keep the flags clean.           
  if(out==0) zout=-1; //check for Zero_Condition                                                   
                                                                         
 //now to pass the flags to the outputs.                                 
  if(zout) out|=0x10; //Zero                                             
  if(cout) out|=0x80; //Carry                                            
                                                                         
  return(out);                                                           
} 

void setup() {
  // put your setup code here, to run once:
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);
  Serial.begin(57600);

  eraseEEPROM();
  programEEPROM();
  // Read and print out the contents of the EERPROM
  Serial.println("Reading EEPROM");
  printContents();
}

void loop() {
  // put your main code here, to run repeatedly:

}
