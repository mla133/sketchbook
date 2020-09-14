
//By: Chuck Kozik

//Notes Timer0 used for timer functions delay(), millis() and micros()
//The Servo Library will not be used
//Therefore, 16 bit timers Timer1, Timer3, Timer4, Timer5 will be used to
//generate pulse outputs. 
//Timer1 will control PB5 (PULSE1)
//Timer3 will control PE4 (PULSE2)
//Timer4 will control PH3 (PULSE3)
//Timer5 will control PL4 (PULSE4)

#include <SPI.h>
#include <Ethernet2.h>
//reference c:ProgramFiles(86)-Arduino-hardware-tools-avr-avr-include-avr
//reference c:ProgramFiles(86)-Arduino-hardware-tools-avr-avr-include-avr-iomxx0_1.h
//be sure Tools-Board is set to correct device

#define RISE_EDGE        1
#define FALL_EDGE        2

#define MAX_FREQ_HZ          (unsigned long)130

#define POF_NORMAL_MODE   0
#define POF_LEAK_MODE     1
#define POF_CLEAR_COUNTS  2


// MAC address from Ethernet shield sticker under board
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x19, 0x4B };
//IPAddress ip(192, 168, 0, 21); // IP address, may need to change depending on network (ip(10, 0, 0, 20);)
EthernetServer server(80);  // create a server at port 80

String HTTP_req;            // stores the HTTP request

void check_MI1_adjust(void);
void check_MI2_adjust(void);
void Ethernet_Control(EthernetClient client); 
int MtrInjOut1 = 11;//PB5 (arduino digital pin # 11)
int MtrInjOut2 = 12;//PB6 (arduino digital pin # 12)
int MtrInjOut3 = 5;//PE4 (arduino digital pin # 2)
int MtrInjOut4 = 2;//PE3 (arduino digital pin # 5)
int MtrInjSol1 = 48;
int MtrInjSol2 = 49;
int MtrInjSol3 = 21;
int MtrInjSol4 = 20;
unsigned long pulse1CountA;
int pulse1State;
unsigned int pulse1Freq;
unsigned int pulse1MaxFreq;
int pulse1OutFunctionReq;
int pulse1OutFunction;
unsigned int pulse1OCRA;
unsigned long pulse2CountA;
int pulse2State;
unsigned int pulse2Freq;
unsigned int pulse2MaxFreq;
int pulse2OutFunctionReq;
int pulse2OutFunction;
unsigned int pulse2OCRA;
unsigned long pulse3CountA;
int pulse3State;
unsigned int pulse3Srate;
unsigned int pulse3Freq;
unsigned int pulse3MaxFreq;
int pulse3OutFunctionReq;
int pulse3OutFunction;
unsigned int pulse3OCRA;
unsigned long pulse4CountA;
int pulse4State;
unsigned int pulse4Srate;
unsigned int pulse4Freq;
unsigned int pulse4MaxFreq;
int pulse4OutFunctionReq;
int pulse4OutFunction;
unsigned int pulse4OCRA;

const char *prompt =
  "    ______  ___       ___   ______\r\n"
  "   |   ___||   \\    /   | /       |\r\n"
  " __|  |__  |    \\  /    ||   _____|\r\n"
  "(__    __) |     \\/     ||  (_____\r\n"
  "   |  |    |  |\\    /|  ||        |\r\n"
  "   |__|    |__| \\__/ |__| \\_______| Technologies\r\n";


void setup() 
{
    pinMode(53, OUTPUT);
    Ethernet.begin(mac);  // initialize Ethernet device

 Serial.begin(115200);

  //Print Device Information
  Serial.print(prompt);
  Serial.println("\nAccuTest Board - Metered Injector Simulator\n");

  Serial.print("MAC Address: ");
  for (byte i = 0; i < sizeof(mac); i++)
  {
    Serial.print(mac[i], HEX);
    
    if(i < (sizeof(mac) - 1))
    {
      Serial.print("-");
    }
    else
    {
           Serial.print("\n");
    }
  }

  while(!Ethernet.begin(mac))
  {
    Serial.println("Failed to Get DHCP Address, Trying Again");
    delay(2000);
  }

  Serial.print("IP Address:  ");
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
      
    server.begin();           // start to listen for clients
    //indicate cause of reset
    //set up for pulse output 1, uses timer 1 
    pulse1CountA = 0;
    pulse1State = RISE_EDGE;
    pulse1Freq = 0;
    pulse1MaxFreq = MAX_FREQ_HZ;
    pinMode(MtrInjOut1, OUTPUT);
    pinMode(MtrInjOut2, OUTPUT);
    pinMode(MtrInjSol1, INPUT);
    pinMode(MtrInjSol2, INPUT);
   //TIMER0 is used for internal Arduino functionality e.g millis()
  //diconnect port pins from timer since they will be used by application 
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
    TCCR1A = 0;
    //set control register B for 256 bit prescale, CTC mode
    TCCR1B = ((1 << WGM12) | (1 << CS12));
    pulse1OCRA = OCR1A = 0x00ff;
    TIMSK1 |= (1<<OCIE1A);//enable interript for Timer/Counter1 Output Compare Match A
    
    //set up for pulse output 2, uses timer 3
    pulse2CountA = 0;
    pulse2State = RISE_EDGE;
    pulse2Freq = 0;
    pulse2MaxFreq = MAX_FREQ_HZ;
    pinMode(MtrInjOut3, OUTPUT);
    pinMode(MtrInjOut4, OUTPUT);
    pinMode(MtrInjSol3, INPUT);
    pinMode(MtrInjSol4, INPUT);
    TCCR3A = 0;
    //set control register B for 256 bit prescale, CTC mode
    TCCR3B = ((1 << WGM32) | (1 << CS32));
    pulse2OCRA = OCR3A = 0x00ff;
    TIMSK3 |= (1<<OCIE3A);//enable interript for Timer/Counter 3 Output Compare Match A
    
     //set up for pulse output 3, uses timer 4
    pulse3CountA = 0;
    pulse3State = RISE_EDGE;
    pulse3Freq = 0;
    pulse3MaxFreq = MAX_FREQ_HZ;
     TCCR4A = 0;
    //set control register B for 256 bit prescale, CTC mode
    TCCR4B = ((1 << WGM42) | (1 << CS42));
    pulse3OCRA = OCR4A = 0x00ff;
    TIMSK4 |= (1<<OCIE4A);//enable interript for Timer/Counter 4 Output Compare Match A
    
     //set up for pulse output 4, uses timer 5
    pulse4CountA = 0;
    pulse4State = RISE_EDGE;
    pulse4Freq = 0;
    pulse4MaxFreq = MAX_FREQ_HZ;
    TCCR5A = 0;
    //set control register B for 256 bit prescale, CTC mode
    TCCR5B = ((1 << WGM52) | (1 << CS52));
    pulse4OCRA = OCR5A = 0x00ff;
    TIMSK5 |= (1<<OCIE5A);//enable interript for Timer/Counter 5 Output Compare Match A
    
    SREG |= 0b1000000;//global interrupt enable
  
}


void loop() 
{
    unsigned long current_millis;
    EthernetClient client;
     
    current_millis = millis();
    check_MI1_adjust();
    check_MI2_adjust();
    check_MI3_adjust();
    check_MI4_adjust();
    Ethernet_Control(client);
}

//Mtr Inj 1
void check_MI1_adjust(void)
{
   unsigned int ocr;
   unsigned int i;
   int _on;
   static bool _was_on = false;
   unsigned int _freq;
   
  _on = digitalRead(MtrInjSol1);
  Serial.println(_on);
  //Serial.println(_down);
  
  if((_on==0) && !_was_on)  //H11L1 inverts
  {
    //solenoid just went active
    _freq = pulse1MaxFreq = 70;    
   //use 8000000 since we are interrupting on rising and falling B  
    ocr = 31250/_freq;//8000000/256 = 31250. 256 is prescale
    TIMSK1 &= ~(1<<OCIE1A); //temporarily disable interrupt
    i++; //delay
    pulse1OCRA = ocr;
    TIMSK1 |= (1<<OCIE1A);//reenable interrupt
    _was_on = true;
    pulse1Freq = _freq;
  } 
  else if(_on==1)
  {
    //solenoid  just went not active
    pulse1Freq = 0;
    _was_on = false;
  }
   
}  
//pulse 2
void check_MI2_adjust(void)
{
   unsigned int ocr;
   unsigned int i;
   int _on;
   static bool _was_on = false;
   unsigned int _freq;
   //int _down;
   
  _on = digitalRead(MtrInjSol2);

  if((_on==0) && !_was_on)  //H11L1 inverts
  {
    //solenoid just went active
    _freq = pulse2MaxFreq = 70;    
   //use 8000000 since we are interrupting on rising and falling B  
    ocr = 31250/_freq;//8000000/256 = 31250. 256 is prescale
    TIMSK3 &= ~(1<<OCIE3A); //temporarily disable interrupt
    i++; //delay
    pulse2OCRA = ocr;
    TIMSK3 |= (1<<OCIE3A);//reenable interrupt
    _was_on = true;
    pulse2Freq = _freq;
  } 
  else if(_on==1) 
  {
    //solenoid  just went not active
    pulse2Freq = 0;
    _was_on = false;
  }
  
}  

//pulse 3
void check_MI3_adjust(void)
{
   unsigned int ocr;
   unsigned int i;
   int _on;
   static bool _was_on = false;
   unsigned int _freq;
   
  _on = digitalRead(MtrInjSol3);
  
  if((_on==0) && !_was_on)  //H11L1 inverts
  {
    //solenoid just went active
    _freq = pulse3MaxFreq = 70;    
   //use 8000000 since we are interrupting on rising and falling B  
    ocr = 31250/_freq;//8000000/256 = 31250. 256 is prescale
    TIMSK4 &= ~(1<<OCIE4A); //temporarily disable interrupt
    i++; //delay
    pulse3OCRA = ocr;
    TIMSK4 |= (1<<OCIE4A);//reenable interrupt
    _was_on = true;
    pulse3Freq = _freq;
  } 
  else if(_on==1)
  {
    //solenoid  just went not active
    pulse3Freq = 0;
    _was_on = false;
  }
   
} 

//pulse 4
void check_MI4_adjust(void)
{
   unsigned int ocr;
   unsigned int i;
   int _on;
   static bool _was_on = false;
   unsigned int _freq;
   //int _down;
   
  _on = digitalRead(MtrInjSol4);
  
  if((_on==0) && !_was_on)  //H11L1 inverts
  {
    //solenoid just went active
    _freq = pulse4MaxFreq = 70;    
   //use 8000000 since we are interrupting on rising and falling B  
    ocr = 31250/_freq;//8000000/256 = 31250. 256 is prescale
    TIMSK5 &= ~(1<<OCIE5A); //temporarily disable interrupt
    i++; //delay
    pulse4OCRA = ocr;
    TIMSK5 |= (1<<OCIE5A);//reenable interrupt
    _was_on = true;
    pulse4Freq = _freq;
  } 
  else if(_on==1)
  {
    //solenoid  just went not active
    pulse4Freq = 0;
    _was_on = false;
  }
  
} 

/**********************************************************************************/
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
                    {
                      pulse1OutFunctionReq = pulse_function_out(HTTP_req);
                    }
                    else if(HTTP_req.indexOf("mtr2.txt")> -1)
                    {
                       pulse2OutFunctionReq = pulse_function_out(HTTP_req);
                    }
                    else if(HTTP_req.indexOf("mtr3.txt")> -1)
                    {
                      pulse3OutFunctionReq = pulse_function_out(HTTP_req);
                    }
                    else if(HTTP_req.indexOf("mtr4.txt")> -1)
                    {
                      pulse4OutFunctionReq = pulse_function_out(HTTP_req);
                    }
                    else if(HTTP_req.indexOf("m1sld.txt") > -1)
                    {
                      pulse1MaxFreq = freq_string_to_int(HTTP_req);
                    }
                    else if(HTTP_req.indexOf("m2sld.txt") > -1)
                    {
                      pulse2MaxFreq = freq_string_to_int(HTTP_req);
                    }
                    else if(HTTP_req.indexOf("m3sld.txt") > -1)
                    {
                      pulse3MaxFreq = freq_string_to_int(HTTP_req);
                    }
                    else if(HTTP_req.indexOf("m4sld.txt") > -1)
                    {
                      pulse4MaxFreq = freq_string_to_int(HTTP_req);
                    }
                    else
                    {  // HTTP request for web page
                        // send web page - contains JavaScript with AJAX calls
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>Metered Injector Control Web Page</title>");
                        client.println("<script>");
                        client.println("function GetPulseCountData() {");
                        client.println(
                            "nocache = \"&nocache=\" + Math.random() * 1000000;"); 
                        client.println("var request = new XMLHttpRequest();");
                        client.println("request.onreadystatechange = function() {");
                        client.println("if (this.readyState == 4) {");
                        client.println("if (this.status == 200) {");
                        client.println("if (this.responseText != null) {");
                        client.println("document.getElementById(\"pulse_data\").innerHTML = this.responseText;");
                        client.println("}}}}");
                        client.println(
                        "request.open(\"GET\", \"ajax_pulse\" + nocache, true);");
                        client.println("request.send(null);");
                        client.println("setTimeout('GetPulseCountData()', 1000);");//client.println("setTimeout('GetPulseCountData()', 1000);");
                        client.println("}");
                        /***/
                        //Meter Valve 1 Selection
                      client.println("function valveSelect1(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    

                      client.println("xmlhttp.open(\"GET\",\"vlv1.txt?q=\"+str,true);");

                      client.println("xmlhttp.send();");
                      client.println("}");//end of function valveSelect1(str) 
                      //Meter Valve 2 Selection
                      client.println("function valveSelect2(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    

                      client.println("xmlhttp.open(\"GET\",\"vlv2.txt?q=\"+str,true);");

                      client.println("xmlhttp.send();");
                      client.println("}");//end of function valveSelect2(str) 
                      //Meter Valve 3 Selection
                      client.println("function valveSelect3(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    

                      client.println("xmlhttp.open(\"GET\",\"vlv3.txt?q=\"+str,true);");

                      client.println("xmlhttp.send();");
                      client.println("}");//end of function valveSelect3(str) 
                      //Meter Valve 4 Selection
                      client.println("function valveSelect4(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    

                      client.println("xmlhttp.open(\"GET\",\"vlv4.txt?q=\"+str,true);");

                      client.println("xmlhttp.send();");
                      client.println("}");//end of function valveSelect4(str) 
                       /***/
                       //Mter 1 pulse Selection
                      client.println("function pulseSelect1(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    

                      client.println("xmlhttp.open(\"GET\",\"mtr1.txt?q=\"+str,true);");

                      client.println("xmlhttp.send();");
                      client.println("}");//end of function pulseSelect1(str) 
                    //Meter 2 Pulse Selection 
                      client.println("function pulseSelect2(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");   
                      client.println("xmlhttp.open(\"GET\",\"mtr2.txt?q=\"+str,true);");

                      client.println("xmlhttp.send();");
                      client.println("}");//end of function pulseSelect2(str) 
                      //Meter 3 Pulse Selection
                       client.println("function pulseSelect3(str) {");
                       client.println("xmlhttp=new XMLHttpRequest();");    
                      client.println("xmlhttp.open(\"GET\",\"mtr3.txt?q=\"+str,true);");

                      client.println("xmlhttp.send();");
                      client.println("}");//end of function pulseSelect3(str) 
                      //Meter 4 Pulse Selection
                       client.println("function pulseSelect4(str) {");
                       client.println("xmlhttp=new XMLHttpRequest();");    
                      client.println("xmlhttp.open(\"GET\",\"mtr4.txt?q=\"+str,true);");

                      client.println("xmlhttp.send();");
                      client.println("}");//end of function pulseSelect4(str) 
                      
                      //start of meter 1 slider() 
                     client.println("function updateSlider1(slideAmount)");
                     client.println("{");
                     client.println("var xmlhttp= new XMLHttpRequest();");  
                     client.println("var sliderDiv = document.getElementById(\"slider1\");");
                     client.println("document.getElementById(\"slider1\").innerHTML= \"Meter 1 = \"+slideAmount+\" Hz\";");
                     client.println("xmlhttp.open(\"GET\",\"m1sld.txt?q=\"+slideAmount,true);");
                     client.println("xmlhttp.send();");
                     client.println("}");
                    //end of meter 1 slider 
                    //start of meter 2 slider() 
                     client.println("function updateSlider2(slideAmount)");
                     client.println("{");
                     client.println("var xmlhttp= new XMLHttpRequest();");  
                     client.println("var sliderDiv = document.getElementById(\"slider2\");");
                     client.println("document.getElementById(\"slider2\").innerHTML= \"Meter 2 = \"+slideAmount+\" Hz\";");
                     client.println("xmlhttp.open(\"GET\",\"m2sld.txt?q=\"+slideAmount,true);");
                     client.println("xmlhttp.send();");
                     client.println("}");
                    //end of meter 2 slider 
                    //start of meter 3 slider() 
                     client.println("function updateSlider3(slideAmount)");
                     client.println("{");
                     client.println("var xmlhttp= new XMLHttpRequest();");  
                     client.println("var sliderDiv = document.getElementById(\"slider3\");");
                     client.println("document.getElementById(\"slider3\").innerHTML= \"Meter 3 = \"+slideAmount+\" Hz\";");
                     client.println("xmlhttp.open(\"GET\",\"m3sld.txt?q=\"+slideAmount,true);");
                     client.println("xmlhttp.send();");
                     client.println("}");
                    //end of meter 3 slider 
                    //start of meter 4 slider() 
                     client.println("function updateSlider4(slideAmount)");
                     client.println("{");
                     client.println("var xmlhttp= new XMLHttpRequest();");  
                     client.println("var sliderDiv = document.getElementById(\"slider4\");");
                     client.println("document.getElementById(\"slider4\").innerHTML= \"Meter 4 = \"+slideAmount+\" Hz\";");
                     client.println("xmlhttp.open(\"GET\",\"m4sld.txt?q=\"+slideAmount,true);");
                     client.println("xmlhttp.send();");
                     client.println("}");
                    //end of meter 4 slider 
                        
                        /****/
                        client.println("</script>");
                        client.println("</head>");
                        client.println("<body onload=\"GetPulseCountData()\">");
                        client.println("<h1>Metered Injector Pulse Data</h1>");
                        client.println("<div id=\"pulse_data\">");
                        client.println("</div>");
                        /****************************************/
                        //Start of Valve Selections
                          //Start table for Meter Valve selections
                       client.println("<br>");
                       client.println("<table style=\"width:40%\">");
                       client.println("<tr>");
                         //select list valve 1 
                       client.println("<td>");
                       client.println("<p>Mtr Inj 1 Valve</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"valve1\" onchange=\"valveSelect1(this.value)\">");
                       client.println("<option value=\"Dig\">Digital 210</option>");
                       client.println("<option value=\"Ana\">Analog</option>");
                       client.println("</select>");
                       client.println("</form>");

                       client.println("</td>");
                       //select list Valve 2
                       client.println("<td>");
                      client.println("<p>Mtr Inj 2 Valve</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"valve2\" onchange=\"valveSelect2(this.value)\">");
                       client.println("<option value=\"Dig\">Digital 210</option>");
                       client.println("<option value=\"Ana\">Analog</option>");
                       client.println("</select>");
                       client.println("</form>");

                       client.println("</td>");
                       //select list Valve 3
                       client.println("<td>");
                       client.println("<p>Mtr Inj 3 Valve</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"valve3\" onchange=\"valveSelect3(this.value)\">");
                       client.println("<option value=\"Dig\">Digital 210</option>");
                       client.println("<option value=\"Ana\">Analog</option>");
                       client.println("</select>");
                       client.println("</form>");

                       client.println("</td>");
                        //select list Valve 4
                      client.println("<td>");
                      client.println("<p>Mtr Inj 4 Valve</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"valve4\" onchange=\"valveSelect4(this.value)\">");
                       client.println("<option value=\"Dig\">Digital 210</option>");
                       client.println("<option value=\"Ana\">Analog</option>");
                       client.println("</select>");
                       client.println("</form>");

                       client.println("</td>");
                       //end od table
                       client.println("</tr");

                       client.println("</table>");
                        //End of Valve Selections
                        /***************************************/
                       //Start table for Meter Pulse selections

                       client.println("<table style=\"width:40%\">");
                       client.println("<tr>");
                         //select list meter 1
                       client.println("<td>");
                       client.println("<p>Mtr Inj 1 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"pulse1\" onchange=\"pulseSelect1(this.value)\">");
                       client.println("<option value=\"A0\">A Lead B</option>");
                       client.println("<option value=\"B1\">B Lead A</option>");
                       client.println("<option value=\"C2\">A Only</option>");
                       client.println("<option value=\"D3\">B Only</option>");
                       client.println("<option value=\"E4\">A Lead B Leak</option>");
                       client.println("<option value=\"F5\">B Lead A Leak</option>");
                       client.println("<option value=\"G6\">A Only Leak</option>");
                       client.println("<option value=\"H7\">B Only Leak</option>");
                       client.println("<option value=\"I8\">Clear Counts</option>");
                       client.println("</select>");
                       client.println("</form>");

                       client.println("</td>");
                       //select list meter 2
                       client.println("<td>");
                       client.println("<p>Mtr Inj 2 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"pulse2\" onchange=\"pulseSelect2(this.value)\">");
                       client.println("<option value=\"A0\">A Lead B</option>");
                       client.println("<option value=\"B1\">B Lead A</option>");
                       client.println("<option value=\"C2\">A Only</option>");
                       client.println("<option value=\"D3\">B Only</option>");
                       client.println("<option value=\"E4\">A Lead B Leak</option>");
                       client.println("<option value=\"F5\">B Lead A Leak</option>");
                       client.println("<option value=\"G6\">A Only Leak</option>");
                       client.println("<option value=\"H7\">B Only Leak</option>");
                       client.println("<option value=\"I8\">Clear Counts</option>");
                       client.println("</select>");
                       client.println("</form>");

                       client.println("</td>");
                       //select list meter 3
                       client.println("<td>");
                       client.println("<p>Mtr Inj 3 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"pulse3\" onchange=\"pulseSelect3(this.value)\">");
                       client.println("<option value=\"A0\">A Lead B</option>");
                       client.println("<option value=\"B1\">B Lead A</option>");
                       client.println("<option value=\"C2\">A Only</option>");
                       client.println("<option value=\"D3\">B Only</option>");
                       client.println("<option value=\"E4\">A Lead B Leak</option>");
                       client.println("<option value=\"F5\">B Lead A Leak</option>");
                       client.println("<option value=\"G6\">A Only Leak</option>");
                       client.println("<option value=\"H7\">B Only Leak</option>");
                       client.println("<option value=\"I8\">Clear Counts</option>");
                       client.println("</select>");
                       client.println("</form>");

                       client.println("</td>");
                        //select list meter 4
                       client.println("<td>");
                       client.println("<p>Mtr Inj 4 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"pulse4\" onchange=\"pulseSelect4(this.value)\">");
                       client.println("<option value=\"A0\">A Lead B</option>");
                       client.println("<option value=\"B1\">B Lead A</option>");
                       client.println("<option value=\"C2\">A Only</option>");
                       client.println("<option value=\"D3\">B Only</option>");
                       client.println("<option value=\"E4\">A Lead B Leak</option>");
                       client.println("<option value=\"F5\">B Lead A Leak</option>");
                       client.println("<option value=\"G6\">A Only Leak</option>");
                       client.println("<option value=\"H7\">B Only Leak</option>");
                       client.println("<option value=\"I8\">Clear Counts</option>");
                       client.println("</select>");
                       client.println("</form>");

                       client.println("</td>");
                       //end od table
                       client.println("</tr");
 
                       client.println("</table>");
                       //start table for slides to set frequency
                       //slider meter 1        

                       client.println("<table style=\"width:70%\">");
                       client.println("<tr>");
                       client.println("<td>");
                       client.println("<p>Mtr Inj 1 Freq</p>");
                       client.println("0 Hz <input id=\"slide\" type=\"range\"");
                       client.println("min=\"0\" max=\"1100\" step=\"8\" value=\"1100\"");
                       client.println("onchange=\"updateSlider1(this.value)\">");
                       client.println("1100 Hz");
                       client.println("<div id=\"slider1\"></div>");
                       client.println("</td>");
                       //end slider meter 1
                       //start meter 2 slide
                       client.println("<td>");
                       client.println("<p>Mtr Inj 2 Freq</p>");
                       client.println("0 Hz <input id=\"slide\" type=\"range\"");
                       client.println("min=\"0\" max=\"1100\" step=\"8\" value=\"1100\"");
                       client.println("onchange=\"updateSlider2(this.value)\">");
                       client.println("1100 Hz");
                       client.println("<div id=\"slider2\"></div>");
                       client.println("</td>");
                       //end meter 2 slide
                       //start meter 3 slide
                       client.println("<td>");
                       client.println("<p>Mtr Inj 3 Freq</p>");
                       client.println("0 Hz <input id=\"slide\" type=\"range\"");
                       client.println("min=\"0\" max=\"1100\" step=\"8\" value=\"1100\"");
                       client.println("onchange=\"updateSlider3(this.value)\">");
                       client.println("1100 Hz");
                       client.println("<div id=\"slider3\"></div>");
                       client.println("</td>");
                       //end meter 3 slide
                       //start meter 4 slide
                       client.println("<td>");
                       client.println("<p>Mtr Inj 4 Freq</p>");
                       client.println("0 Hz <input id=\"slide\" type=\"range\"");
                       client.println("min=\"0\" max=\"1100\" step=\"8\" value=\"1100\"");
                       client.println("onchange=\"updateSlider4(this.value)\">");
                       client.println("1100 Hz");
                       client.println("<div id=\"slider4\"></div>");
                       client.println("</td>");
                       //end meter 4 slide
                       client.println("</tr");
                       client.println("</table>");
                       
                       client.println("</body>");
                        //*******
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
   cl.println("Mtr Inj 1:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(0);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(0);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1Freq);
   cl.println("</td>");
   cl.println("</tr>");
   //Meter 2
   cl.println("<tr>"); 
   cl.println("<td>");
   cl.println("Mtr Inj 2:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(0);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(0);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2Freq);
   cl.println("</td>");
   cl.println("</tr>");
   //Meter 3
   cl.println("<tr>"); 
   cl.println("<td>");
   cl.println("Mtr Inj 3:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse3CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(0);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(0);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse3Freq);
   cl.println("</td>");
   cl.println("</tr>");
    //Meter 4
   cl.println("<tr>"); 
   cl.println("<td>");
   cl.println("Mtr Inj 4:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse4CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(0);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(0);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse4Freq);
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
   }
   if(valid)
     freq = (unsigned int)my_string.toInt();
   return(freq);
}

/****************************************************************************/
int pulse_function_out(String HTTP_req_arg)
{
 
  int func=0;
  #ifdef chcuk
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
 #endif
  return(func);
}

/**********************************************************************************/
#define P1_RISE   PORTB |=0x20;//set PB5 high(11);
#define P1_FALL   PORTB &=0xdf;//clear PB5 (11)
 
ISR(TIMER1_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
  switch(pulse1State)
  {
    case RISE_EDGE:
      if((pulse1Freq > 0) && ((PORTL && 0x02) == 0x00))//PL1 is solenoid 1
      {
        OCR1A = pulse1OCRA; 
        pulse1State = FALL_EDGE;
        P1_RISE;//generates rising edge of a pulse at output
        pulse1CountA+=1Ul;
      }  
      else
        OCR1A = 0x0c35;// a default value
       break;
          
      case FALL_EDGE:
        pulse1State = RISE_EDGE;//generates falling edge of a pulse at output
        P1_FALL;
      break;

    
  }     
}  
/***************************************************************************************/
#define P2_RISE   PORTB |= 0x40;//set PB6 high(12)
#define P2_FALL   PORTB &= 0xbf;//set PB5 high(11); 

ISR(TIMER3_COMPA_vect)  
{  
  //interrupt flag automatically cleared upon interrupt execution
   switch(pulse2State)
  {
    case RISE_EDGE:
      if((pulse2Freq > 0) && ((PORTL && 0x01) == 0x00))//PL0 is solenoid 2
      {
        OCR3A = pulse2OCRA; //used to set frequency 
        pulse2State = FALL_EDGE;//setup for next time around
        P2_RISE;//generates rising edge of a pulse at output
        pulse2CountA+=1Ul;
      }  
      else
        OCR3A = 0x0c35;// a default value
       break;
          
      case FALL_EDGE:
        pulse2State = RISE_EDGE;
        P2_FALL;//generates falling edge of a pulse output
      break;

    
  }     
} 
/***************************************************************************************/
#define P3_RISE   PORTE |= 0x08;//set PE4 high;
#define P3_FALL   PORTE &= 0xf7;// clear PE4 

ISR(TIMER4_COMPA_vect)  
{
    //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    switch(pulse3State)
  {
    case RISE_EDGE:
      if((pulse3Freq > 0) && ((PORTD && 0x01) == 0x00)) //PD0 is inj3 solenoid
      {
        OCR4A = pulse3OCRA; //used to set frequency 
        pulse3State = FALL_EDGE;//setup for next time around
        P3_RISE;//generates rising edge of a pulse at output
        pulse3CountA+=1Ul;
      }  
      else
        OCR4A = 0x0c35;// a default value
       break;
          
      case FALL_EDGE:
        pulse3State = RISE_EDGE;
        P3_FALL;//generates falling edge of a pulse output
      break;    
  }     
 
}  
/****************************************************************************************/
#define P4_RISE   PORTE |=0x10;//set PE3 high
#define P4_FALL   PORTE &= 0xef;//clear PE3

ISR(TIMER5_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
     switch(pulse4State)
  {
    case RISE_EDGE:
      if((pulse4Freq > 0) && ((PORTD && 0x02) == 0x00))//PD1 is solenoid 4
      {
        OCR5A = pulse4OCRA; //used to set frequency 
        pulse4State = FALL_EDGE;//setup for next time around
        P4_RISE;//generates rising edge of a pulse at output
        pulse4CountA+=1Ul;
      }  
      else
        OCR5A = 0x0c35;// a default value
       break;
          
      case FALL_EDGE:
        pulse4State = RISE_EDGE;
        P4_FALL;//generates falling edge of a pulse output
      break;
   
  }     
  
}
