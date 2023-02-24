// Arduino Mega using all four of its Serial ports
// (Serial, injSerial, Serial2, Serial3),

// Not all pins on the Mega and Mega 2560 support change interrupts,
// so only the following can be used for RX when using SoftwareSerial pins:
// 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69
#ifndef BYTE
  #define BYTE unsigned char
#endif

#define MAX_BUF 25
#define MAX_TMP_BUF 40
#define ADDR_SIZE 3
BYTE STX = 0x02;
BYTE ETX = 0x03;
BYTE PAD = 0x7F;
BYTE NUL = 0x00;

#include <SoftwareSerial.h>
SoftwareSerial injSerial(3,2,true); // RX, TX, INVERT --> Arduino inverts TTL signals

char response[MAX_BUF];
char tempBuf[MAX_TMP_BUF];
char add_buf[ADDR_SIZE];
char command[MAX_BUF];
String buf = "";
bool buf_recv = false;
double NRT_1, NRT_2, NRT_3, NRT_4, pulse_1, pulse_2, pulse_3, pulse_4 = 0;
int sol_1,sol_2, sol_3, sol_4, pump_1,pump_2, pump_3, pump_4;
enum { OFF, ON };
int pipe, buf_size, inj_address;
String numStr = "";

void setup() {
  Serial.begin(9600);
  injSerial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }
  Serial.println("Hello Computer");
  injSerial.print("Hello Hercules");
  buf.reserve(MAX_BUF);
}
void loop() {
  if(readSerial()){
    strcpy(tempBuf, buf.c_str());
    
    if (buf_recv) {
      buf = "";
      buf_recv = false; 
    }
    
    memcpy(add_buf, tempBuf+1, 2);
    inj_address = atoi(add_buf);

    // Parse buffer, supply response buffer
    if ( strncmp ("IN", tempBuf+3, 2) == 0 ) {
      //Serial.print("IN found: ");
      if (inj_address < 400) {
        NRT_1 += 0.013;     //Serial.print(NRT_1, 3); Serial.print(" | ");
        pulse_1 += 65.0;    //Serial.print(pulse_1, 0); Serial.print(" | ");
        sol_1 = ON;         //Serial.println(sol_1);
        
      }

      memset(response,0,sizeof(response));
      response[0] = STX;
      strcat(response, add_buf);
      strcat(response, "OK");
      response[strlen(response)] = ETX;
      response[strlen(response)] = GetLRC(response+1, strlen(response-1));
      response[strlen(response)] = PAD;
      //Serial.println(GetLRC(response+1, strlen(response-1)));
      //Serial.println(strlen(response));
      Serial.println(response);
      injSerial.write(response);

    }
    else if ( strncmp ("TS", tempBuf+3, 2) == 0) {
      if (inj_address < 400) {
        Serial.println("TS found: ");
        sol_1 = OFF;
        strcat(response, "TS ");
        numStr = String(NRT_1, 3); 
        strcat(response, numStr.c_str());
        strcat(response, " 0000");
        strcat(response, 0x03);
        strcat(response, GetLRC(response, strlen(response)));
        Serial.println(response);
        //injSerial.print(s_response);
      }
    }

  }
  memset(response,0,MAX_BUF);
  numStr = "";
}

bool readSerial() {
  while (injSerial.available()) {
    // get the new byte:
    char inChar = (char)injSerial.read();
    // add it to the recv_msg:
    buf += inChar;
    // if the incoming character is a newline, set a flag so the main loop can do something about it:
    if((inChar == 0x0A) || (inChar == 0x7F)) {
      buf_recv = true;
    }
  }
  return buf_recv;   
}

unsigned char GetLRC(unsigned char* pbData, int iCount) {
  unsigned char chLRC = 0;
  while (iCount > 0) {
    chLRC ^= *pbData++;
    iCount--;
  }
  return chLRC;
}
