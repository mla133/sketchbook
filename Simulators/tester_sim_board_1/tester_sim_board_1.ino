
//By: Chuck Kozik

//Notes Timer0 used for timer functions delay(), millis() and micros()
//The Servo Library will not be used
//Therefore, 16 bit timers Timer1, Timer3, Timer4, Timer5 will be used to
//generate pulse outputs. 
//Timer1 will control PB5 (PULSE1 A) and (Pulse1 B) PB6
//Timer3 will control PE4 (PULSE2 A) and (Pulse2 B) PH3
//Timer4 will control PH3 (PULSE3 A) and (Pulse3 B) PH4
//Timer5 will control PL4 (PULSE4 A) and (Pulse4 B) PL3

#include <SPI.h>
#include <Ethernet.h>

#define A_LEAD_RISE_A        1
#define A_LEAD_RISE_B        2
#define A_LEAD_FALL_A        3
#define A_LEAD_FALL_B        4
#define B_LEAD_RISE_B        6
#define B_LEAD_RISE_A        7
#define B_LEAD_FALL_B        8
#define B_LEAD_FALL_A        9

#define MAX_FREQ_HZ          (unsigned long)500
#define MIN_FREQ_ANA_VALVE   (unsigned long) 3

#define POF_A_LEAD_B      0
#define POF_B_LEAD_A      1
#define POF_A_ONLY        2
#define POF_B_ONLY        3
#define POF_A_LEAD_B_LEAK 4
#define POF_B_LEAD_A_LEAK 5
#define POF_A_ONLY_LEAK   6
#define POF_B_ONLY_LEAK   7
#define POF_CLEAR_COUNTS  8

#define LAST_RISE_EDGE_NONE   0
#define LAST_RISE_EDGE_A      1
#define LAST_RISE_EDGE_B      2

#define VALVE_TYPE_DIGITAL          0
#define VALVE_TYPE_ANALOG           1

// MAC address from Ethernet shield sticker under board, IPAddress set to static IP
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xA1, 0x9E };
IPAddress ip(192,168,76,2);
EthernetServer server(80);  // create a server at port 80

String HTTP_req;            // stores the HTTP request

void check_pulse1_adjust(void);
void check_pulse2_adjust(void);

//Pulse Out 1
int PULSE_OUT_1A = 11;//PB5 (arduino digital pin # 11)
int PULSE_OUT_1B = 12;//PB6 (arduino digital pin # 12)
int PULSE_1_UPSTREAM = 48;
int PULSE_1_DOWNSTREAM = 49;
unsigned long pulse1CountA, pulse1CountB, pulse1Error;
int pulse1State;
unsigned int pulse1Freq, pulse1MaxFreq;
unsigned long pulse1AdjustWaitTime, pulse1AdjustTimeout;
int pulse1OutFunctionReq, pulse1OutFunction, pulse1LastRise;
unsigned int pulse1OCRA, pulse1ValveType;

//Pulse Out 2
int PULSE_OUT_2A = 5;//PE4 (arduino digital pin # 2)
int PULSE_OUT_2B = 2;//PE3 (arduino digital pin # 5)
int PULSE_2_UPSTREAM = 21;
int PULSE_2_DOWNSTREAM = 20;
unsigned long pulse2CountA, pulse2CountB, pulse2Error;
int pulse2State;
unsigned int pulse2Freq, pulse2MaxFreq;
unsigned long pulse2AdjustWaitTime, pulse2AdjustTimeout;
int pulse2OutFunctionReq, pulse2OutFunction, pulse2LastRise;
unsigned int pulse2OCRA, pulse2ValveType;

void setup() 
{
  //to use Ethernet 2 set pin 53 as an output
  pinMode(53, OUTPUT);
  Serial.begin(115200);
  
  //Print Device Information
  Serial.println("\nAccuTest Board - Meter Simulator\n");
  
  Serial.print("MAC Address: ");
  for (byte i = 0; i < sizeof(mac); i++)
  {
    Serial.print(mac[i], HEX);
    
    if(i < (sizeof(mac) - 1))
      Serial.print("-");
    else
      Serial.print("\n");
  }

  Ethernet.begin(mac,ip);
  Ethernet.setLocalIP(ip);
    
  Serial.print("IP Address:  ");
  for (byte i = 0; i < 4; i++)
  {
    Serial.print(Ethernet.localIP()[i], DEC);
    if(i < 3)
      Serial.print(".");
    else
      Serial.print("\n");
  }
    
  server.begin();           // start to listen for clients
  //indicate cause of reset
  //set up for pulse output 1, uses timer 1 
  pulse1CountA = 0;
  pulse1CountB = 0;
  pulse1Error = 0;
  pulse1State = A_LEAD_RISE_A;
  pulse1Freq = 0;
  pulse1AdjustWaitTime = 500;//time in milliseconds
  pulse1AdjustTimeout  = millis() + pulse1AdjustWaitTime;
  pulse1MaxFreq = MAX_FREQ_HZ;
  pulse1OutFunctionReq = POF_A_LEAD_B;
  pulse1OutFunction = POF_A_LEAD_B;
  pulse1LastRise = LAST_RISE_EDGE_NONE;
  pulse1ValveType = VALVE_TYPE_DIGITAL;
  pinMode(PULSE_OUT_1A, OUTPUT);
  pinMode(PULSE_OUT_1B, OUTPUT);
  pinMode(PULSE_1_UPSTREAM, INPUT);
  pinMode(PULSE_1_DOWNSTREAM, INPUT);
  
  //TIMER0 is used for internal Arduino functionality e.g millis()
  //diconnect port pins from timer since they will be used by application 
  TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
  TCCR1A = 0;
  //set control register B for 64 bit prescale, CTC mode
  TCCR1B = ((1 << WGM12) | (1 << CS11) | (1 << CS10));
  pulse1OCRA = OCR1A = 0x00ff;
  TIMSK1 |= (1<<OCIE1A);//enable interript for Timer/Counter1 Output Compare Match A
  
  //set up for pulse output 2, uses timer 3
  pulse2CountA = 0;
  pulse2CountB = 0;
  pulse2Error = 0;
  pulse2State = A_LEAD_RISE_A;
  pulse2Freq = 0;
  pulse2AdjustWaitTime = 500;//time in milliseconds
  pulse2AdjustTimeout  = millis() + pulse2AdjustWaitTime;
  pulse2MaxFreq = MAX_FREQ_HZ;
  pulse2OutFunctionReq = POF_A_LEAD_B;
  pulse2OutFunction = POF_A_LEAD_B;
  pulse2LastRise = LAST_RISE_EDGE_NONE;
  pulse2ValveType = VALVE_TYPE_DIGITAL;
  pinMode(PULSE_OUT_2A, OUTPUT);
  pinMode(PULSE_OUT_2B, OUTPUT);
  pinMode(PULSE_2_UPSTREAM, INPUT);
  pinMode(PULSE_2_DOWNSTREAM, INPUT);
  TCCR3A = 0;
  //set control register B for 64 bit prescale, CTC mode
  TCCR3B = ((1 << WGM32) | (1 << CS31) | (1 << CS30));
  pulse2OCRA = OCR3A = 0x00ff;
  TIMSK3 |= (1<<OCIE3A);//enable interript for Timer/Counter 3 Output Compare Match A
  
  SREG |= 0b1000000;//global interrupt enable
} // end setup()

void loop() 
{
  unsigned long current_millis;
  EthernetClient client;
  
  current_millis = millis();
  if((long)(current_millis - pulse1AdjustTimeout) >= 0)
  {
    if((pulse1OutFunction == POF_CLEAR_COUNTS) || (pulse1OutFunctionReq == POF_CLEAR_COUNTS))
    {
      pulse1CountA = 0uL;
      pulse1CountB = 0uL;
      pulse1Error = 0uL;
    }   
    if(pulse1ValveType == VALVE_TYPE_DIGITAL)
    {
      check_pulse1_adjust();
      if(pulse1Freq < 8)
        pulse1AdjustWaitTime = 750;//time in milliseconds
      else if(pulse1Freq < 12)
        pulse1AdjustWaitTime = 250;//time in milliseconds
      else
        pulse1AdjustWaitTime = 30;//time in milliseconds
    }
    else
    {
      //Ana_Valve_Pulse1();
      pulse1AdjustWaitTime = 100;//time in milliseconds
    }
    pulse1AdjustTimeout  = current_millis + pulse1AdjustWaitTime;//maintain last timeout   
  }
  
  if((long)(current_millis - pulse2AdjustTimeout) >= 0)
  {
    if((pulse2OutFunction == POF_CLEAR_COUNTS) || (pulse2OutFunctionReq == POF_CLEAR_COUNTS))
    {      
      pulse2CountA = 0uL;
      pulse2CountB = 0uL;
      pulse2Error = 0uL;
    } 
  
    if(pulse2ValveType == VALVE_TYPE_DIGITAL)
    {
      check_pulse2_adjust();
      if(pulse2Freq < 8)
        pulse2AdjustWaitTime = 750;//time in milliseconds
      else if(pulse2Freq < 12)
        pulse2AdjustWaitTime = 250;//time in milliseconds
      else
        pulse2AdjustWaitTime = 30;//time in milliseconds
    }
    else
    {
      //Ana_Valve_Pulse2();
      pulse2AdjustWaitTime = 100;//time in milliseconds
    }
    pulse2AdjustTimeout  = current_millis + pulse2AdjustWaitTime;//maintain last timeout   
  }
    
  Ethernet_Control(client);
} // end loop()

//pulse 1
void check_pulse1_adjust(void)
{
  bool pulse_increase;
  bool pulse_decrease; 
  bool _new_max_freq;
  unsigned int ocr;
  unsigned int i;
  int _up;
  int _down;
  
  pulse_increase = false;
  pulse_decrease = false;
  _new_max_freq = false; 
  _up = digitalRead(PULSE_1_UPSTREAM);
  _down = digitalRead(PULSE_1_DOWNSTREAM);
  
  //H11L1 inverts
  if(_up && !_down)
    pulse_decrease = true; //illegal state  
  else if(!_up && !_down)
    pulse_increase = true;
  else if(_up && _down)  
    pulse_decrease = true;
  
  if(pulse1Freq == 0)
    pulse1OutFunction =  pulse1OutFunctionReq;  
  if((pulse1OutFunction == POF_A_ONLY_LEAK) || (pulse1OutFunction == POF_B_ONLY_LEAK) ||
  (pulse1OutFunction == POF_A_LEAD_B_LEAK) || (pulse1OutFunction == POF_B_LEAD_A_LEAK))
  {
    pulse_increase = true;
  } 
  
  if(pulse_increase) 
  {
    if(pulse1Freq == 0)
      pulse1Freq = 2;
    else if(pulse1Freq < 20)
      pulse1Freq += 2;
    else
      pulse1Freq += 4;
  } 
  else if(pulse_decrease)
  {
    if(pulse1Freq <= 3)
      pulse1Freq = 0;
    else if(pulse1Freq < 20)
      pulse1Freq -= 2;
    else 
      pulse1Freq -= 4;  
  }  
  
  if(pulse1Freq > pulse1MaxFreq)
  {
    pulse1Freq = pulse1MaxFreq;//the maximum limit   
    _new_max_freq = true;
  }
  
  if(pulse_increase || pulse_decrease || _new_max_freq)
  {  
    ocr = 0x00ff;//default setting
    if(pulse1Freq > 0)
    {
      //use 4000000 since we are interrupting on every every edge of the A and B pulse  
      ocr = 62500/pulse1Freq;//4000000/64 = 62500. 64 is prescale
    }
     
    TIMSK1 &= ~(1<<OCIE1A); //temporarily disable interrupt
    i++; //delay
    pulse1OCRA = ocr;
    TIMSK1 |= (1<<OCIE1A);//reenable interrupt
  }  
}
  
//pulse 2
void check_pulse2_adjust(void)
{
  bool pulse_increase;
  bool pulse_decrease;
  bool _new_max_freq; 
  unsigned int ocr;
  unsigned int i;
  
  int _up;
  int _down;
  
  pulse_increase = false;
  pulse_decrease = false;
  _new_max_freq = false;
  _up = digitalRead(PULSE_2_UPSTREAM);
  _down = digitalRead(PULSE_2_DOWNSTREAM);
  
  //H11L1 inverts
  if(_up && !_down)
    pulse_decrease = true; //illegal state  
  else if(!_up && !_down)
    pulse_increase = true;
  else if(_up && _down)  
    pulse_decrease = true;
  
  if(pulse2Freq == 0)
    pulse2OutFunction =  pulse2OutFunctionReq;
  if((pulse2OutFunction == POF_A_ONLY_LEAK) || (pulse2OutFunction == POF_B_ONLY_LEAK) ||
  (pulse2OutFunction == POF_A_LEAD_B_LEAK) || (pulse2OutFunction == POF_B_LEAD_A_LEAK))
  {
    pulse_increase = true;
  } 
  
  if(pulse_increase)
  {
    if(pulse2Freq == 0)
      pulse2Freq = 2;
    else if(pulse2Freq < 20)
      pulse2Freq += 2;
    else
      pulse2Freq += 4;
  } 
  else if(pulse_decrease)
  {
  if(pulse2Freq <= 3)
    pulse2Freq = 0;
  else if(pulse2Freq < 20)
    pulse2Freq -= 2;
  else 
    pulse2Freq -= 4;  
  }  
  
  if(pulse2Freq > pulse2MaxFreq)
  {
    pulse2Freq = pulse2MaxFreq;//the maximum limit 
    _new_max_freq = true;
  }
  
  //pulse2Freq = 3;
  if(pulse_increase || pulse_decrease || _new_max_freq)
  {  
    if(pulse2Freq < 8)
      pulse2AdjustWaitTime = 750;//time in milliseconds
    else if(pulse2Freq < 12)
      pulse2AdjustWaitTime = 250;//time in milliseconds
    else
      pulse2AdjustWaitTime = 50;//time in milliseconds
    ocr = 0x00ff;//default setting
    if(pulse2Freq > 0)
    {
      //use 4000000 since we are interrupting on every every edge of the A and B pulse  
      ocr = 62500/pulse2Freq;//4000000/64 = 62500. 64 is prescale 
    }
    
    TIMSK3 &= ~(1<<OCIE3A); //temporarily disable interrupt
    i++; //delay
    pulse2OCRA = ocr;
    TIMSK3 |= (1<<OCIE3A);//reenable interrupt
  }
}  

void Ethernet_Control(EthernetClient client) 
{  
    client = server.available();  // try to get client
    int i =0;
    
    if (client)
    {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: keep-alive");
                    client.println();
                
                    // AJAX request for switch state
                    if (HTTP_req.indexOf("ajax_pulse") > -1)
                    {
                        // read switch state and analog input
                        GetAjaxData(client);
                    }
                    else if(HTTP_req.indexOf("mtr1.txt")> -1)
                      pulse1OutFunctionReq = pulse_function_out(HTTP_req);
                    else if(HTTP_req.indexOf("mtr2.txt")> -1)
                       pulse2OutFunctionReq = pulse_function_out(HTTP_req);
                    else if(HTTP_req.indexOf("m1sld.txt") > -1)                   
                      pulse1MaxFreq = freq_string_to_int(HTTP_req);
                    else if(HTTP_req.indexOf("m2sld.txt") > -1)
                      pulse2MaxFreq = freq_string_to_int(HTTP_req);
                    else
                    {  // HTTP request for web page
                        // send web page - contains JavaScript with AJAX calls
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>Meter 1-4 Control Web Page</title>");
                        client.println("<script>");
                        client.println("function GetPulseCountData() {");
                        client.println("nocache = \"&nocache=\" + Math.random() * 1000000;"); 
                        client.println("var request = new XMLHttpRequest();");
                        client.println("request.onreadystatechange = function() {");
                        client.println("if (this.readyState == 4) {");
                        client.println("if (this.status == 200) {");
                        client.println("if (this.responseText != null) {");
                        client.println("document.getElementById(\"pulse_data\").innerHTML = this.responseText;");
                        client.println("}}}}");
                        client.println("request.open(\"GET\", \"ajax_pulse\" + nocache, true);");
                        client.println("request.send(null);");
                        client.println("setTimeout('GetPulseCountData()', 1000);");
                        client.println("}");
                        
                        //Mter 1 pulse Selection
                        client.println("function pulseSelect1(str) {");
                        client.println("xmlhttp=new XMLHttpRequest();");    
                        client.println("xmlhttp.open(\"GET\",\"mtr1.txt?q=\"+str,true);");
                        client.println("xmlhttp.send();");
                        client.println("}");
                        
                        //Meter 2 Pulse Selection 
                        client.println("function pulseSelect2(str) {");
                        client.println("xmlhttp=new XMLHttpRequest();");   
                        client.println("xmlhttp.open(\"GET\",\"mtr2.txt?q=\"+str,true);");
                        client.println("xmlhttp.send();");
                        client.println("}");
                      
                        client.println("</script>");
                        client.println("</head>");
                        client.println("<body onload=\"GetPulseCountData()\">");
                        client.println("<h1>Meter Pulse Data (1-4)</h1>");
                        client.println("<div id=\"pulse_data\">");
                        client.println("</div>");

                       //Start table for Meter Pulse selections
                       client.println("<table>");
                       client.println("<tr>");
                       
                       //select list meter 1
                       client.println("<td>");
                       client.println("<p>Meter 1 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">");                        
                       client.println("<button type=\"button\" value=\"E4\" onclick=\"pulseSelect1(this.value)\">A->B Leak</button><br>");
                       client.println("<button type=\"button\" value=\"F5\" onclick=\"pulseSelect1(this.value)\">B->A Leak</button><br>");
                       client.println("<button type=\"button\" value=\"A0\" onclick=\"pulseSelect1(this.value)\">Stop Leak</button><br>");
                       client.println("<button type=\"button\" value=\"I8\" onclick=\"pulseSelect1(this.value)\">Clear</button><br>");
                       client.println("</form>");
                       client.println("</td>");
                       
                       //select list meter 2
                       client.println("<td>");
                       client.println("<p>Meter 2 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<button type=\"button\" value=\"E4\" onclick=\"pulseSelect2(this.value)\">A->B Leak</button><br>");
                       client.println("<button type=\"button\" value=\"F5\" onclick=\"pulseSelect2(this.value)\">B->A Leak</button><br>");
                       client.println("<button type=\"button\" value=\"A0\" onclick=\"pulseSelect2(this.value)\">Stop Leak</button><br>");
                       client.println("<button type=\"button\" value=\"I8\" onclick=\"pulseSelect2(this.value)\">Clear</button><br>");
                       client.println("</form>");
                       client.println("</td>");
 
                       client.println("</tr");
                       client.println("</table>");                       
                       client.println("</body>");
                       client.println("</html>");
                    }
                    // display received HTTP request on serial port
                    HTTP_req = "";            // finished with request, empty string
                    break;//while loop
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n')
                {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r')
                {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data, 1 millisecond
        client.stop(); // close the connection
    } // end if (client)
}

/********************************************************************************/

void GetAjaxData(EthernetClient cl)
{
   cl.println("<table style=\"width:50%\">"); 
   cl.println("<tr>");
   cl.println("<td>");
   cl.println(" ");
   cl.println("</td>");
   cl.println("<td>");
   cl.println("<h3>A Counts</h3>");
   cl.println("</td>");
   cl.println("<td>");
   cl.println("<h3>B Counts</h3>");
   cl.println("</td>");
   cl.print("<td>");
   cl.println("<h3>Errors</h3>");
   cl.println("</td>");
   cl.print("<td>");
   cl.println("<h3>Pulse Freq</h3>");
   cl.println("</td>");
   cl.println("</tr>");
   //Meter 1
   cl.println("<td>");
   cl.println("Meter 1:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1CountB);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1Error);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1Freq);
   cl.println("</td>");
   cl.println("</tr>");
   //Meter 2
   cl.println("<tr>"); 
   cl.println("<td>");
   cl.println("Meter 2:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2CountB);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2Error);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2Freq);
   cl.println("</td>");
   cl.println("</tr>");
   //close the table
   cl.println("</table>");

}
/********************************************************************************/

unsigned int freq_string_to_int(String HTTP_req_arg)
{
   String my_string;//object
   char char_val = 0;
   int index = 17;
   unsigned int freq = 1200;
   bool valid = false;
    
   char_val = HTTP_req_arg.charAt(index);//get first character
   //Serial.println(char_val);
   while((char_val >='0') && (char_val<='9'))
   {
     my_string += char_val;//append
     valid = true;
     index++;
     char_val = HTTP_req_arg.charAt(index);//get successive characters
     //Serial.println(char_val);
   }
   if(valid)
     freq = (unsigned int)my_string.toInt();
   return(freq);
}

/****************************************************************************/
int pulse_function_out(String HTTP_req_arg)
{
  int func;
  func = POF_A_LEAD_B;//default
  if(HTTP_req_arg.indexOf("A0") > -1)
   func = POF_A_LEAD_B;
  else if(HTTP_req_arg.indexOf("B1") > -1)
    func = POF_B_LEAD_A; 
  else if(HTTP_req_arg.indexOf("C2") > -1)
    func = POF_A_ONLY; 
  else if(HTTP_req_arg.indexOf("D3") > -1)
    func = POF_B_ONLY; 
  else if(HTTP_req_arg.indexOf("E4") > -1)
    func = POF_A_LEAD_B_LEAK; 
  else if(HTTP_req_arg.indexOf("F5") > -1)
    func = POF_B_LEAD_A_LEAK; 
  else if(HTTP_req_arg.indexOf("G6") > -1)
    func = POF_A_ONLY_LEAK; 
  else if(HTTP_req_arg.indexOf("H7") > -1)
    func = POF_B_ONLY_LEAK;
   else if(HTTP_req_arg.indexOf("I8") > -1)
    func = POF_CLEAR_COUNTS;  
  return(func);
}

/**********************************************************************************/
#define P1_RISE_A   PORTB |=0x20;//set PB5 high(11);
#define P1_RISE_B   PORTB |= 0x40;//set PB6 high(12)
#define P1_FALL_A   PORTB &=0xdf;//clear PB5 (11)
#define P1_FALL_B   PORTB &= 0xbf;//set PB5 high(11); 
ISR(TIMER1_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    if((pulse1OutFunction == POF_A_LEAD_B) || (pulse1OutFunction == POF_A_LEAD_B_LEAK))
    {
      switch(pulse1State)
      {
        case A_LEAD_RISE_A:
          if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = A_LEAD_RISE_B;
            P1_RISE_A;
            pulse1CountA+=1Ul;
            if(pulse1LastRise == LAST_RISE_EDGE_A)
              pulse1Error++;
            pulse1LastRise = LAST_RISE_EDGE_A;
          }  
        break;
        case A_LEAD_RISE_B:
          pulse1State = A_LEAD_FALL_A;
          P1_RISE_B;
          pulse1CountB+=1Ul;
          pulse1LastRise = LAST_RISE_EDGE_B;
        break;
        case A_LEAD_FALL_A:
          pulse1State = A_LEAD_FALL_B;
          P1_FALL_A;
        break;
        case A_LEAD_FALL_B:
          pulse1State = A_LEAD_RISE_A;
          P1_FALL_B
          pulse1OutFunction = pulse1OutFunctionReq; 
        break;
        default://same as A_LEAD_RISE_A
         if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = A_LEAD_RISE_B;
            P1_RISE_A;
            pulse1CountA+=1Ul;
            if(pulse1LastRise == LAST_RISE_EDGE_A)
              pulse1Error++;
            pulse1LastRise = LAST_RISE_EDGE_A;
          }  
        break;
      }
    }
    else if((pulse1OutFunction == POF_B_LEAD_A) || (pulse1OutFunction == POF_B_LEAD_A_LEAK))
    {
      switch(pulse1State)
      {
        case B_LEAD_RISE_B:
          if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = B_LEAD_RISE_A;
            P1_RISE_B;
            pulse1CountB+=1Ul;
            if(pulse1LastRise == LAST_RISE_EDGE_B)
              pulse1Error++;
            pulse1LastRise = LAST_RISE_EDGE_B;
          }  
        break;
        case B_LEAD_RISE_A:
          pulse1State = B_LEAD_FALL_B;
          P1_RISE_A;
          pulse1CountA+=1Ul;
          pulse1LastRise = LAST_RISE_EDGE_A;
        break;
        case B_LEAD_FALL_B:
          pulse1State = B_LEAD_FALL_A;
          P1_FALL_B;
        break;
        case B_LEAD_FALL_A:
          pulse1State = B_LEAD_RISE_B;
          P1_FALL_A;
          pulse1OutFunction = pulse1OutFunctionReq; 
        break;
        default://same as B_LEAD_RISE_B
          if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = B_LEAD_RISE_A;
            P1_RISE_B;
            pulse1CountB+=1Ul;
            if(pulse1LastRise == LAST_RISE_EDGE_B)
              pulse1Error++;
            pulse1LastRise = LAST_RISE_EDGE_B;
          }  
        break;
      }
     }
      else if((pulse1OutFunction == POF_A_ONLY) || (pulse1OutFunction == POF_A_ONLY_LEAK))
      {
        switch(pulse1State)
        {
          case A_LEAD_RISE_A:
            if(pulse1Freq > 0)
            {
              OCR1A = pulse1OCRA; 
              pulse1State = A_LEAD_RISE_B;
              P1_RISE_A;
              pulse1CountA+=1Ul;
              if(pulse1LastRise == LAST_RISE_EDGE_A)
                pulse1Error++;
              pulse1LastRise = LAST_RISE_EDGE_A;
            }  
          break;
          case A_LEAD_RISE_B:
            pulse1State = A_LEAD_FALL_A;
           //no B pulse
           break;
          case A_LEAD_FALL_A:
            pulse1State = A_LEAD_FALL_B;
            P1_FALL_A;
          break;
          case A_LEAD_FALL_B:
            pulse1State = A_LEAD_RISE_A;
            //no B pulse
            pulse1OutFunction = pulse1OutFunctionReq; 
          break;
          default://same as A_LEAD_RISE_A
            if(pulse1Freq > 0)
            {
              OCR1A = pulse1OCRA; 
              pulse1State = A_LEAD_RISE_B;
              P1_RISE_A;
              pulse1CountA+=1Ul;
              if(pulse1LastRise == LAST_RISE_EDGE_A)
                pulse1Error++;
              pulse1LastRise = LAST_RISE_EDGE_A;
            }  
          break;
        }
      }
     else if((pulse1OutFunction == POF_B_ONLY) || (pulse1OutFunction == POF_B_ONLY_LEAK))
      {
        switch(pulse1State)
        {
          case B_LEAD_RISE_B:
            if(pulse1Freq > 0)
            {
              OCR1A = pulse1OCRA; 
              pulse1State = B_LEAD_RISE_A;
              P1_RISE_B;
              pulse1CountB+=1Ul;
              if(pulse1LastRise == LAST_RISE_EDGE_B)
                pulse1Error++;
              pulse1LastRise = LAST_RISE_EDGE_B;
            }  
          break;
          case B_LEAD_RISE_A:
            pulse1State = B_LEAD_FALL_B;
            //pulse1Error++;
            //NO A PULSE
           break;
          case B_LEAD_FALL_B:
            pulse1State = B_LEAD_FALL_A;
            P1_FALL_B;
          break;
          case B_LEAD_FALL_A:
            pulse1State = B_LEAD_RISE_B;
            //NO A PULSE
            pulse1OutFunction = pulse1OutFunctionReq; 
          break;
          default://same as B_LEAD_RISE_B
            if(pulse1Freq > 0)
            {
              OCR1A = pulse1OCRA; 
              pulse1State = B_LEAD_RISE_A;
              P1_RISE_B;
              pulse1CountB+=1Ul;
              if(pulse1LastRise == LAST_RISE_EDGE_B)
                pulse1Error++;
              pulse1LastRise = LAST_RISE_EDGE_B;
            }  
          break;
      }
     }
      
      else if(pulse1OutFunction == POF_CLEAR_COUNTS)
      {
        pulse1CountA = 0uL;
        pulse1CountB = 0uL;
        pulse1Error = 0uL;
        //pulse1LastRise = LAST_RISE_EDGE_NONE;
        pulse1OutFunction = pulse1OutFunctionReq;
      }
}  
/***************************************************************************************/
#define P2_RISE_A   PORTE |= 0x08;//set PE3 high
#define P2_RISE_B   PORTE |=0x10;//set PE4 high;
#define P2_FALL_A   PORTE &= 0xf7;//clear PE3
#define P2_FALL_B   PORTE &= 0xef;// clear PE4

ISR(TIMER3_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    if((pulse2OutFunction == POF_A_LEAD_B) || (pulse2OutFunction == POF_A_LEAD_B_LEAK))
    {
      switch(pulse2State)
      {
        case A_LEAD_RISE_A:
          if(pulse2Freq > 0)
          {
            OCR3A = pulse2OCRA; 
            pulse2State = A_LEAD_RISE_B;
            P2_RISE_A;
            pulse2CountA+=1Ul;
            if(pulse2LastRise == LAST_RISE_EDGE_A)
              pulse2Error++;
            pulse2LastRise = LAST_RISE_EDGE_A;
          }  
        break;
        case A_LEAD_RISE_B:
          pulse2State = A_LEAD_FALL_A;
          P2_RISE_B;
          pulse2CountB+=1Ul;
          pulse2LastRise = LAST_RISE_EDGE_B;
        break;
        case A_LEAD_FALL_A:
          pulse2State = A_LEAD_FALL_B;
          P2_FALL_A;
        break;
        case A_LEAD_FALL_B:
          pulse2State = A_LEAD_RISE_A;
          P2_FALL_B;
          pulse2OutFunction = pulse2OutFunctionReq; 
        break;
        default://same as A_LEAD_RISE_A
         if(pulse2Freq > 0)
          {
            OCR3A = pulse2OCRA; 
            pulse2State = A_LEAD_RISE_B;
            P2_RISE_A;
            pulse2CountA+=1Ul;
            if(pulse2LastRise == LAST_RISE_EDGE_A)
              pulse2Error++;
            pulse2LastRise = LAST_RISE_EDGE_A;
          }  
        break;
      }
    }
    else if((pulse2OutFunction == POF_B_LEAD_A) || (pulse2OutFunction == POF_B_LEAD_A_LEAK))
    {
      switch(pulse2State)
      {
        case B_LEAD_RISE_B:
          if(pulse2Freq > 0)
          {
            OCR3A = pulse2OCRA; 
            pulse2State = B_LEAD_RISE_A;
            P2_RISE_B;
            pulse2CountB+=1Ul;
            if(pulse2LastRise == LAST_RISE_EDGE_B)
              pulse2Error++;
            pulse2LastRise = LAST_RISE_EDGE_B;
          }  
        break;
        case B_LEAD_RISE_A:
          pulse2State = B_LEAD_FALL_B;
          P2_RISE_A;
          pulse2CountA+=1Ul;
          pulse2LastRise = LAST_RISE_EDGE_A;
        break;
        case B_LEAD_FALL_B:
          pulse2State = B_LEAD_FALL_A;
          P2_FALL_B;
        break;
        case B_LEAD_FALL_A:
          pulse2State = B_LEAD_RISE_B;
          P2_FALL_A;
          pulse2OutFunction = pulse2OutFunctionReq; 
        break;
        default://same as B_LEAD_RISE_B
          if(pulse2Freq > 0)
          {
            OCR3A = pulse2OCRA; 
            pulse2State = B_LEAD_RISE_A;
            P2_RISE_B;
            pulse2CountB+=1Ul;
            if(pulse2LastRise == LAST_RISE_EDGE_B)
              pulse2Error++;
            pulse2LastRise = LAST_RISE_EDGE_B;
          }  
        break;
      }
     }
      else if((pulse2OutFunction == POF_A_ONLY) || (pulse2OutFunction == POF_A_ONLY_LEAK))
      {
        switch(pulse2State)
        {
          case A_LEAD_RISE_A:
            if(pulse2Freq > 0)
            {
              OCR3A = pulse2OCRA; 
              pulse2State = A_LEAD_RISE_B;
              P2_RISE_A;
              pulse2CountA+=1Ul;
              if(pulse2LastRise == LAST_RISE_EDGE_A)
                pulse2Error++;
              pulse2LastRise = LAST_RISE_EDGE_A;
            }  
          break;
          case A_LEAD_RISE_B:
            pulse2State = A_LEAD_FALL_A;
           //no B pulse
           break;
          case A_LEAD_FALL_A:
            pulse2State = A_LEAD_FALL_B;
            P2_FALL_A;
          break;
          case A_LEAD_FALL_B:
            pulse2State = A_LEAD_RISE_A;
            //no B pulse
            pulse2OutFunction = pulse2OutFunctionReq; 
          break;
          default://same as A_LEAD_RISE_A
            if(pulse2Freq > 0)
            {
              OCR3A = pulse2OCRA; 
              pulse2State = A_LEAD_RISE_B;
              P2_RISE_A;
              pulse2CountA+=1Ul;
              if(pulse2LastRise == LAST_RISE_EDGE_A)
                pulse2Error++;
              pulse2LastRise = LAST_RISE_EDGE_A;
            }  
          break;
        }
      }
     else if((pulse2OutFunction == POF_B_ONLY) || (pulse2OutFunction == POF_B_ONLY_LEAK))
      {
        switch(pulse2State)
        {
          case B_LEAD_RISE_B:
            if(pulse2Freq > 0)
            {
              OCR3A = pulse2OCRA; 
              pulse2State = B_LEAD_RISE_A;
              P2_RISE_B;
              pulse2CountB+=1Ul;
              if(pulse2LastRise == LAST_RISE_EDGE_B)
                pulse2Error++;
              pulse2LastRise = LAST_RISE_EDGE_B;
            }  
          break;
          case B_LEAD_RISE_A:
            pulse2State = B_LEAD_FALL_B;
            //pulse2Error++;
            //NO A PULSE
           break;
          case B_LEAD_FALL_B:
            pulse2State = B_LEAD_FALL_A;
            P2_FALL_B;
          break;
          case B_LEAD_FALL_A:
            pulse2State = B_LEAD_RISE_B;
            //NO A PULSE
            pulse2OutFunction = pulse2OutFunctionReq; 
          break;
          default://same as B_LEAD_RISE_B
            if(pulse2Freq > 0)
            {
              OCR3A = pulse2OCRA; 
              pulse2State = B_LEAD_RISE_A;
              P2_RISE_B;
              pulse2CountB+=1Ul;
              if(pulse2LastRise == LAST_RISE_EDGE_B)
                pulse2Error++;
              pulse2LastRise = LAST_RISE_EDGE_B;
            }  
          break;
      }
     }
      
      else if(pulse2OutFunction == POF_CLEAR_COUNTS)
      {
        pulse2CountA = 0uL;
        pulse2CountB = 0uL;
        pulse2Error = 0uL;
        //pulse2LastRise = LAST_RISE_EDGE_NONE;
        pulse2OutFunction = pulse2OutFunctionReq;
      }
} 
