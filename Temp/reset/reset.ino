void(* resetFunc) (void) = 0;
 
void setup() {     
  Serial.begin(9600);
  Serial.println("How to Reset Arduino Programmatically");
  Serial.println("www.TheEngineeringProjects.com");
  delay(200);
}

void loop() 
{
  Serial.println("A");
  delay(1000);               
  Serial.println("B");
  delay(1000);               
  Serial.println("Now we are Resetting Arduino Programmatically");
  Serial.println();
  delay(1000);
  resetFunc();
  Serial.println("Arrduino will never reach there.");
 
}