#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

ESP8266WiFiMulti wifiMulti;

ESP8266WebServer server(81);

const int sensors_count = 8;

void handleIndex() {
  int values_of_sensors[sensors_count];
  String s = "<meta http-equiv=\"refresh\" content=\"0; url=http://188.126.25.95:5000/profile\" />";
  server.send(200, "text/html", s);
  DynamicJsonDocument doc(1024);
  double distance = 0;
  // Sending the request
  doc["type"] = "request";
  serializeJson(doc,Serial);
  // Reading the response
  boolean messageReady = false;
  String message = "";
  while(messageReady == false) {
    if(Serial.available()) {
      message = Serial.readString();
      messageReady = true;
    }
  }

  HTTPClient http;
  
  http.begin("http://188.126.25.95:5000/insert");
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(message);
  String payload = http.getString();

  Serial.println(httpCode);
  Serial.println(payload);

  http.end();

  delay(5000);
}

void setup() {
  wifiMulti.addAP(username, password);
  Serial.begin(9600);
  while(wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(WiFi.localIP());

  server.on("/",handleIndex);
  server.begin();
}

void loop() {
  server.handleClient();
}
