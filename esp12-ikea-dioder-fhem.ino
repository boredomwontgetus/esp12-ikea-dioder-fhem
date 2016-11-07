#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

// GPIO definitions for LEDs
#define red 14
#define green 13
#define blue 12

// Max values for PWM
#define colorOnRed 1024
#define colorOnGreen 1024
#define colorOnBlue 1024

// Hardcode WiFi parameters
const char* ssid = ".....";
const char* password = ".....";

// Define TCP Server on port 5577
WiFiServer server(5577);
WiFiClient client = server.available();

// HTTP Push Update
// curl -F "image=@firmware.bin" <esp-ip>:<port>/update
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  
  Serial.begin(115200);
  WiFi.begin(ssid,password);
  Serial.println("");
  
  //Wait for connection
  digitalWrite(red, HIGH);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  delay(10);

  digitalWrite(red, LOW);
  digitalWrite(green, HIGH);
  delay(3000);

  Serial.print("Connected to "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());
  
  digitalWrite(green, LOW);

  // Start the TCP server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // For push flashing only
  httpServer.handleClient();

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  // Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  int buflen = 8;
  byte inputbuf[buflen];
  client.readBytes(inputbuf, buflen);
  client.flush();
  
  /*
  // DEBUG
  Serial.print("Byte 0: "); Serial.println(inputbuf[0]);
  Serial.print("byte array length: "); Serial.println(sizeof(inputbuf));
  Serial.print("last byte: ");  Serial.println(inputbuf[sizeof(inputbuf) - 1]);
  */
  
  // Find startbyte (LW12 connection of FHEM's Wifilight module)  
  int startbyte;
  for (int i = 0; i < buflen; i++) {
    if (inputbuf[i] == 86 and inputbuf[i + 4] == 170) {
      startbyte = i;
    }
    /*
    // DEBUG
    Serial.print(i); Serial.print(": "); Serial.println(inputbuf[i]);
    */
  }

  int redval = map(inputbuf[startbyte + 1], 0, 255, 0, colorOnRed);
  int greenval = map(inputbuf[startbyte + 2], 0, 255, 0, colorOnGreen);
  int blueval = map(inputbuf[startbyte + 3], 0, 255, 0, colorOnBlue);

  /*
  // DEBUG
  Serial.print("redval: ");  Serial.println(redval);
  Serial.print("greenval: ");  Serial.println(greenval);
  Serial.print("blueval: ");  Serial.println(blueval);
  */

  analogWrite(red, redval);
  analogWrite(green, greenval);
  analogWrite(blue, blueval);

  
  client.flush();
  delay(1);
  //Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed  


  
}


