#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xA1, 0x5A };
IPAddress ip(192,168,76,51);
EthernetServer server(80);

const int outputLED = 53;


void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();

  pinMode(outputLED, OUTPUT);
  digitalWrite(outputLED, HIGH);
}

void loop() {
  EthernetClient client = server.available();
  if(client) 
  {
    boolean currentLineIsBlank =true;
    while(client.connected()) 
    {
      if(client.available()) 
      {
        char c = client.read();
        Serial.write(c);
        if (c=='n' && currentLineIsBlank) 
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<title>Example Webserver</title>");
          client.println("<p>Hello World!</p>");
          client.println("</html>");
          break;
        }
        if(c == 'n') 
        {
          currentLineIsBlank = true;
        } 
        else if (c != 'r') 
        {
            currentLineIsBlank = false;
        }
      }
    }
      delay(1);
      client.stop();
  }
}
