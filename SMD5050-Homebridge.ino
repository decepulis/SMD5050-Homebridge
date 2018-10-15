// based on: https://www.esp8266.com/viewtopic.php?f=11&t=12259

#include <ESP8266WiFi.h>

#define pinR D8
#define pinG D7
#define pinB D6
#define led  D0

///// WiFi SETTINGS /////////////////
WiFiServer server(80); //Set server port
const char* ssid = "***";
const char* password = "***";
IPAddress ip(192, 168, 1, 100);   // set a fixed IP for the NodeMCU
IPAddress gateway(192, 168, 1, 1); // Your router IP
IPAddress subnet(255, 255, 255, 0); // Subnet mask
//////////////////////////////////////

String setHex;
int    setR, setG, setB;
int    stateR, stateB, stateG, stateV, statePower;

String readString;

void WiFiStart() {
  digitalWrite(led,LOW);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("_");
  }
  Serial.println();
  Serial.println("Done");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  digitalWrite(led,HIGH);

  server.begin();
}


void displayRGB(int targetR, int targetG, int targetB) {
  int steps = 100;
  
  float stepR = (targetR-stateR)/steps;
  float stepG = (targetG-stateG)/steps;
  float stepB = (targetB-stateB)/steps;

  // 1 second transition
  for (int i=0; i<steps; i++) {
    stateR = stateR+stepR;
    stateG = stateG+stepG;
    stateB = stateB+stepB;

    analogWrite(pinR, stateR);
    analogWrite(pinG, stateG);
    analogWrite(pinB, stateB);
    
    delay(500/steps);  
  }

  analogWrite(pinR, targetR);
  analogWrite(pinG, targetG);
  analogWrite(pinB, targetB);

  stateR = targetR;
  stateB = targetB;
  stateG = targetG;

  serialPrint();
}

void hex2display() {
  statePower = 1;

  // parse hex and store in RGB
  long number = (long) strtol( &setHex[0], NULL, 16);
  
  setR = map(number >> 16,       0, 255, 0, 1023);
  setG = map(number >> 8 & 0xFF, 0, 255, 0, 1023);
  setB = map(number & 0xFF,      0, 255, 0, 1023);

  // update brightness variable
  float vR = roundf(setR/10.23);
  float vG = roundf(setB/10.23);
  float vB = roundf(setG/10.23);
  stateV = max(max(vR, vG), vB);

  // update strip
  displayRGB(setR, setG, setB);
}

void serialPrint() {
  Serial.print("Status on/off: ");
  Serial.println(statePower);
  Serial.print("RGB color: ");
  Serial.print(stateR);
  Serial.print(".");
  Serial.print(stateG);
  Serial.print(".");
  Serial.println(stateB);
  Serial.print("Hex color: ");
  Serial.println(setHex);
  Serial.print("Brightness: ");
  Serial.println(stateV);
  Serial.println("");
}


void setup() {
  Serial.begin(9600);
  delay(1);

  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(led,  OUTPUT);

  setHex = "F8F1FF";
  hex2display();

  WiFi.mode(WIFI_STA);
  WiFiStart();
}

void loop() {
WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  while (client.connected() && !client.available()) {
    delay(1);
  }
  
  //Respond on certain Homebridge HTTP requests
  if (client) {
    while (client.connected()) {
      if (client.available()) {
         char c = client.read(); // read the next character
         if (readString.length() < 100) { readString += c; } // if to catch mistakes
  
         // newline? time to figure out what we just read.
         if ( c == '\n') {
            Serial.print("Request: ");
            Serial.print(readString);
  
            // Roger, roger.
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
          
            if (readString.indexOf("on") > 0) {
              hex2display(); 
            } 
            else if (readString.indexOf("off") > 0) {
              statePower = 0;
              displayRGB(0,0,0);
            }
            else if (readString.indexOf("set") > 0) {
               setHex = readString.substring(9, 15);
               hex2display();
            }
            else if (readString.indexOf("status") > 0) {
              client.println(statePower);
            }
            else if (readString.indexOf("bright") > 0) {
              client.println(stateV);
            }
            else if (readString.indexOf("color") > 0) {
              client.println(setHex);
            }
  
            // clean up!
            delay(1);
            while (client.read() >= 0); // clear the buffer...
            client.stop();
            readString.remove(0);
         }
      }
    }
  }
}
