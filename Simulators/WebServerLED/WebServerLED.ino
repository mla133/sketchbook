#include <SPI.h>
#include <Ethernet.h>

// MAC address for shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xA1, 0x5A};
IPAddress ip(192,168,76,51);
EthernetServer server(80); // Using port 80
int led = 53; // LED attached to pin 7

void setup() {
  pinMode(led, OUTPUT); // Led set as an output
  Ethernet.begin(mac,ip); // Start the Ethernet shield
  server.begin();
  Serial.begin(9600); // Start serial communication
  Serial.println("Server address:"); // Print server address
  // (Arduino shield)
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    String buffer = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read(); // Read from the Ethernet shield
        buffer += c; // Add character to string buffer
        // Client sent request, now waiting for response
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK"); // HTTP response
          client.println("Content-Type: text/html");
          client.println(); // HTML code
          client.println("<TITLE>LED Controller</TITLE>");
          client.println("<FORM>LED #53 <INPUT type=\"submit\" name=\"status\" value=\"ON\">");
          client.println("<INPUT type=\"submit\" name=\"status\" value=\"OFF\"></FORM>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
          buffer = "";
        }
        else if (c == '\r') { // Command from webpage
          // Did the on button get pressed
          if (buffer.indexOf("GET /?status=ON") >= 0)
            digitalWrite(led, HIGH);
          // Did the off button get pressed
          if (buffer.indexOf("GET /?status=OFF") >= 0)
            digitalWrite(led, LOW);
        }
        else {
          currentLineIsBlank = false;
        }
      }
    }
    client.stop(); // End server
  }
}
