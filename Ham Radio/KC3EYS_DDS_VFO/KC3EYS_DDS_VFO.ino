/*
Main code by Richard Visokey AD7C - www.ad7c.com
Revision 2.0 - November 6th, 2013
Revision 2.1 - Matt Allen KC3EYS --> Removed Rotary switch code, updated to serial LCD scren libs
*/

// Include the library code
#include <SoftwareSerial.h>

//Setup some items
#define W_CLK 8   // Pin 8 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 9   // Pin 9 - connect to freq update pin (FQ)
#define DATA 10   // Pin 10 - connect to serial data load pin (DATA)
#define RESET 11  // Pin 11 - connect to reset pin (RST) 
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }
#define TX_PIN 3
#define LED_PIN 13
#define ANALOG_VOLTAGE_PIN 1

int_fast32_t rx=7025000; // Starting frequency of VFO
int_fast32_t rx2=1; // variable to hold the updated frequency
int_fast32_t increment = 10000; // starting VFO update increment in HZ.
int buttonstate = 0;
int buttonstate1 = 0;
int buttonstate2 = 0;
String hertz = "10 KHz";
int  hertzPosition = 5;
byte ones,tens,hundreds,thousands,tenthousands,hundredthousands,millions ;  //Placeholders
String freq; // string to hold the frequency
int_fast32_t timepassed = millis(); // int to hold the arduino miilis since startup

const char *splash =
" _  __   _____   ____    ______  __    __   _____ \r\n"
"| |/ /  / ____| |___ \  |  ____| \ \  / /  / ____|\r\n"
"| ' /  | |        __) | | |__     \ \/ /  | (___  \r\n"
"|  <   | |       |__ <  |  __|     \  /    \___ \ \r\n"
"| . \  | |____   ___) | | |____    |  |    ____) |\r\n"
"|_|\_\  \_____| |____/  |______|   |__|   |_____/ \r\n";

SoftwareSerial lcd = SoftwareSerial(255, TX_PIN);

void setup() {
  
  Serial.begin(9600);
  Serial.print(splash);
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, HIGH);
  lcd.begin(9600);
  delay(100);
  lcd.write(12);                // Clear
  lcd.write(17);                // Turn backlight on
  delay(5);                     // Required delay (for the clear...)
  lcd.print("KC3EYS");          // First line
  lcd.write(13);                // Form feed
  lcd.print("AD9850 DDS VFO");  // Second line
  delay(3000);                  // Wait 3 seconds
  Serial.println("LCD initialized...");
        
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT); 
  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);             // this pulse enables serial mode on the AD9850 - Datasheet page 12.

  // Connect to a button that goes to GND on push
  pinMode(A0,INPUT);            
  digitalWrite(A0,HIGH);
  pinMode(A1,INPUT);
  digitalWrite(A1,HIGH);
  pinMode(A2,INPUT);
  digitalWrite(A2,HIGH);
}

void loop() 
{
    //rx = SerialInput();

    if (rx != rx2)
    {    
          sendFrequency(rx);
  	      rx2 = rx;
    }
      
    buttonstate = digitalRead(A0);
    if(buttonstate == LOW) 
    {
          setincrement();        
    }
    buttonstate1 = digitalRead(A1);
    if(buttonstate1 == LOW) 
    {
          rx = rx + increment;        
    }
    buttonstate2 = digitalRead(A2);
    if(buttonstate2 == LOW) 
    {
          rx = rx - increment;        
    }
    
    VFOdisplay();
}

// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency(double frequency) 
{  
  int32_t freq = frequency * 4294967295/125000000;  // note 125 MHz clock on 9850.  You can make 'slight' tuning variations here by adjusting the clock frequency.
  for (int b=0; b<4; b++, freq>>=8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
}

// transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte(byte data)
{
  for (int i=0; i<8; i++, data>>=1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

void setincrement(){
  if(increment == 10){increment = 50; hertz = "50 Hz"; hertzPosition=5;}
  else if (increment == 50){increment = 100;  hertz = "100 Hz"; hertzPosition=4;}
  else if (increment == 100){increment = 500; hertz="500 Hz"; hertzPosition=4;}
  else if (increment == 500){increment = 1000; hertz="1 Khz"; hertzPosition=6;}
  else if (increment == 1000){increment = 2500; hertz="2.5 Khz"; hertzPosition=4;}
  else if (increment == 2500){increment = 5000; hertz="5 Khz"; hertzPosition=6;}
  else if (increment == 5000){increment = 10000; hertz="10 Khz"; hertzPosition=5;}
  else if (increment == 10000){increment = 100000; hertz="100 Khz"; hertzPosition=4;}
  else if (increment == 100000){increment = 1000000; hertz="1 Mhz"; hertzPosition=6;}  
  else{increment = 10; hertz = "10 Hz"; hertzPosition=5;};  
}


void VFOdisplay()
{
    millions = int(rx/1000000);
    hundredthousands = ((rx/100000)%10);
    tenthousands = ((rx/10000)%10);
    thousands = ((rx/1000)%10);
    hundreds = ((rx/100)%10);
    tens = ((rx/10)%10);
    ones = ((rx/1)%10);
    lcd.print("                ");
    
    if (millions > 9){setCursor(0,1);}
    else{setCursor(0,2);}
    
    // Top Row VFO Readout
    lcd.print(millions);
    lcd.print(".");
    lcd.print(hundredthousands);
    lcd.print(tenthousands);
    lcd.print(thousands);
    lcd.print(".");
    lcd.print(hundreds);
    lcd.print(tens);
    lcd.print(ones);
    lcd.print(" Mhz  ");
    
    // Bottom Row VFO Increment
    setCursor(1,0);
    lcd.print("                ");
    setCursor(1, hertzPosition); 
    lcd.print(hertz); 
    delay(200); // Adjust this delay to speed up/slow down the button menu scroll speed.
    timepassed = millis();
}

void setCursor(uint8_t row, uint8_t col)
{
    int position;
    if(row==0) position = 128;
    else if (row==1) position = 148;
    
    position = position + col;
    lcd.write(position);    
}

int SerialInput()
{
    char incomingByte;
    while (Serial.available() > 0) 
    {
      incomingByte = Serial.read();
      if (incomingByte == '\n') break;   // exit after newline char
      if (incomingByte == -1) continue;  // if no chars in buffer...Serial.read returns -1...
      rx *= 1000;
      rx += (incomingByte - 48);
    }
    return rx;
}
