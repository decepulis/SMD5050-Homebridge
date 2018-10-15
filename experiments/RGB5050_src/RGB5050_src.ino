// NodeMCU RGB-Controller for Homebridge & HomeKit (Siri)
// https://www.esp8266.com/viewtopic.php?f=11&t=12259

#include <ESP8266WiFi.h>

#define redPin 13 //D7 - Red channel
#define grnPin 12 //D6 - Green channel
#define bluPin 14 //D5 - Blue channel

WiFiServer server(80); //Set server port

String readString;           //String to hold incoming request
String hexString = "000000"; //Define inititial color here (hex value), 080100 would be a calm warmtone i.e.

int state;

int r, g, b, V;

float R, G, B, x;

///// WiFi SETTINGS - Replace with your values /////////////////
const char* ssid = "***";
const char* password = "***";
IPAddress ip(192, 168, 1, 100);   // set a fixed IP for the NodeMCU
IPAddress gateway(192, 168, 1, 1); // Your router IP
IPAddress subnet(255, 255, 255, 0); // Subnet mask
////////////////////////////////////////////////////////////////////

void WiFiStart() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet); //Set a fixed IP. You can comment this out and set it in your router instead.
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("_");
  }
  Serial.println();
  Serial.println("Done");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  server.begin();
}

void allOff() {
  state = 0;
  analogWrite(redPin, 0);
  analogWrite(grnPin, 0);
  analogWrite(bluPin, 0);
}

//Write requested hex-color to the pins (10bit pwm)
void setHex() {
  state = 1;
  long number = (long) strtol( &hexString[0], NULL, 16);
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
  r = map(r, 0, 255, 0, 1023);  //added for 10bit pwm
  g = map(g, 0, 255, 0, 1023);  //added for 10bit pwm
  b = map(b, 0, 255, 0, 1023);  //added for 10bit pwm
  analogWrite(redPin, (r));
  analogWrite(grnPin, (g));
  analogWrite(bluPin, (b));
}

//Compute current brightness value
void getV() {
  R = roundf(r / 10.23); //for 10bit pwm, was (r/2.55);
  G = roundf(g / 10.23); //for 10bit pwm, was (g/2.55);
  B = roundf(b / 10.23); //for 10bit pwm, was (b/2.55);
  x = max(R, G);
  V = max(x, B);
}

//For serial debugging only
void showValues() {
  Serial.print("Status on/off: ");
  Serial.println(state);
  Serial.print("RGB color: ");
  Serial.print(r);
  Serial.print(".");
  Serial.print(g);
  Serial.print(".");
  Serial.println(b);
  Serial.print("Hex color: ");
  Serial.println(hexString);
  getV();
  Serial.print("Brightness: ");
  Serial.println(V);
  Serial.println("");
}

void setup() {
  Serial.begin(9600);
  delay(1);
  pinMode(redPin, OUTPUT);  //declaration added
  pinMode(grnPin, OUTPUT);  //declaration added
  pinMode(bluPin, OUTPUT);  //declaration added
  setHex(); //Set initial color after booting. Value defined above
  WiFi.mode(WIFI_STA);
  WiFiStart();
  showValues(); //Uncomment for serial output
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
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
        }
        if (c == '\n') {
          Serial.print("Request: "); //Uncomment for serial output
          Serial.println(readString); //Uncomment for serial output
          //Send reponse:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          //On:
          if (readString.indexOf("on") > 0) {
            setHex();
            showValues();
          }
          //Off:
          if (readString.indexOf("off") > 0) {
            allOff();
            showValues();
          }
          //Set color:
          if (readString.indexOf("set") > 0) {
            hexString = "";
            hexString = (readString.substring(9, 15));
            setHex();
            showValues();
          }
          //Status on/off:
          if (readString.indexOf("status") > 0) {
            client.println(state);
          }
          //Status color (hex):
          if (readString.indexOf("color") > 0) {
            client.println(hexString);
          }
          //Status brightness (%):
          if (readString.indexOf("bright") > 0) {
            getV();
            client.println(V);
          }
          delay(1);
          while (client.read() >= 0);  //added: clear remaining buffer to prevent ECONNRESET
          client.stop();
          readString.remove(0);
        }
      }
    }
  }
}
