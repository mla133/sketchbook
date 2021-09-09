// Created by ArduinoGetStarted.com
// This example code is in the public domain
// Tutorial page: https://arduinogetstarted.com/tutorials/arduino-send-email

#include <SPI.h>
#include <Ethernet.h>

// replace the MAC address below by the MAC address printed on a sticker on the Arduino Shield 2
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xA1, 0x2D };
//IPAddress ip(192,168,181,78);

EthernetClient client;
EthernetServer server(80);

// EVENT-NAME = arduino_event
// YOUR-KEY  = dlOZX5EiFShYRw5kgIxarM

int    HTTP_PORT   = 80;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "maker.ifttt.com";
//String PATH_NAME   = "/trigger/EVENT-NAME/with/key/YOUR-KEY"; // change your EVENT-NAME and YOUR-KEY
String PATH_NAME   = "/trigger/arduino_event/with/key/dlOZX5EiFShYRw5kgIxarM"; // change your EVENT-NAME and YOUR-KEY

char queryString[60];

//String queryString = "?value1=26&value2=70";

void setup() {
  Serial.begin(9600); 

  // initialize the Ethernet shield using DHCP:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address using DHCP");
    while(true);
  }
  
  Serial.print("Local IP Address:  ");
  for (byte i = 0; i < 4; i++)
  {
    Serial.print(Ethernet.localIP()[i], DEC);
    if(i < 3)
    {
      Serial.print(".");
    }
    else
    {
      Serial.print("\n");
    }
  }

    sprintf(queryString, "?value1=%2X:%2X:%2X:%2X:%2X:%2X&value2=%d.%d.%d.%d", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], Ethernet.localIP()[0],Ethernet.localIP()[1],Ethernet.localIP()[2],Ethernet.localIP()[3]);
    
  // connect to web server on port 80:
  if(client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header

    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {// if not connected:
    Serial.println("connection failed");
  }
}

void loop() {

}
