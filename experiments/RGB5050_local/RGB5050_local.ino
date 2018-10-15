#define led D0            // Led in NodeMCU at pin GPIO16 (D0).
#define red D8    //set the r,g,b, pin for your esp8266
#define green D7
#define blue D6


void setup() {
pinMode(led, OUTPUT);    // LED pin as output.
pinMode(red, OUTPUT);
pinMode(green, OUTPUT);
pinMode(blue, OUTPUT);

analogWrite(led, 0);
analogWrite(red, 0);
analogWrite(green, 0);
analogWrite(blue, 0);
}
void loop() {

analogWrite(red, 128);
analogWrite(blue, 0);
delay(500);
analogWrite(red, 0);
analogWrite(green, 128);
delay(500);
analogWrite(green, 0);
analogWrite(blue, 128);
delay(500);
}

/*
#define red D8    //set the r,g,b, pin for your esp8266
#define green D7
#define blue D6

void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
}

void loop() { 
  analogWrite(red,128);
  analogWrite(green,128);
  analogWrite(blue,128);
  delay(1000);
  analogWrite(red,255);
  analogWrite(green,255);
  analogWrite(blue,255);
  delay(1000);
  analogWrite(red,0);
  analogWrite(green,0);
  analogWrite(blue,0);
  delay(1000);
}
*/
