#include <Arduino.h>
#include "dataWebPage.h"
#include <WiFi.h>
#include <FS.h>
#include <WebServer.h>
#include <LittleFS.h>

// WiFi credentials
const char* ssid = "Bean-Team-ESP";
const char* password = "";

WebServer server(80);


String temperatureValue = "0.0";
String pressureValue = "0.0";
String temperatureSetpoint = "0.0";
String pressureSetpoint = "0.0";

void dataWebPage::init() {
  Serial.begin(115200);
  
  // Set up the Access Point 
  WiFi.softAP(ssid, password);

  if(!LittleFS.begin()){
    Serial.println("An error occured for LittleFS");
    return;
  }

  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/brewData.html", "r");

    // error check
    if (!file) {
      server.send(404, "text/plain", "File not found");
      return;
    }
    
    // stream the page
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/data", HTTP_GET, []() {
    // send data as JSON
    String json = "{";
    json += "\"temp\":" + temperatureValue + ",";
    json += "\"pressure\":" + pressureValue + ",";
    json += "\"tempSet\":" + temperatureSetpoint + ",";
    json += "\"pressureSet\":" + pressureSetpoint;
    json += "}";
    server.send(200, "application/json", json);
  });

  // start the server
  server.begin();
}

void dataWebPage::update(float temp, float pressure, float tempSetPoint, float pressureSetPoint){
  
  // parse floats into strings
  temperatureValue = String(temp, 1);
  pressureValue = String(pressure, 1);
  temperatureSetpoint = String(tempSetPoint, 1);
  pressureSetpoint = String(pressureSetPoint, 1);

  server.handleClient();
}