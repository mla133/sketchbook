#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xA2, 0x24 };
IPAddress ip(192,168,181,71);
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
          for (byte i = 0; i < 4; i++)
          {
             client.print(Ethernet.localIP()[i], DEC);
             if(i<3)
             {
              client.print(".");
             }
             else
             {
              client.println("");
             }
          }
          client.print("<h1>Analogue Values</h1>");
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
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
