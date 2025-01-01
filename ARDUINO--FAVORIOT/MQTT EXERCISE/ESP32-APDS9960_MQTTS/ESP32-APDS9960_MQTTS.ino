#include <WiFi.h>
#include <MQTT.h>
#include <NetworkClientSecure.h>
#include <Adafruit_APDS9960.h>
#include "FavoriotCA_MQTT.h"

const char ssid[]= "YOUR_WIFI_SSID";
const char password[] = "YOUR_WIFI_PASSWORD";
const char deviceDeveloperID[] = "YOUR_DEVICE_DEVELOPER_ID";
const char deviceAccessToken[] = "YOUR_DEVICE_ACCESS_TOKEN";
const char publishTopic[] = "/v2/streams";
const char statusTopic[] = "/v2/streams/status";
const char rpcTopic[] = "/v2/rpc";  // To Get data from dashboard
const char* clientID = "@XK-Esp32-PUBLISH";

Adafruit_APDS9960 apds;
unsigned long lastMillis = 0;
NetworkClientSecure client;
MQTTClient mqtt(4096);

void connectToWiFI();
void connectToFavoriotWiFi();
void SendData();

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi '" + String(ssid) + "' ...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("...");
    delay (250);
  }
  Serial.println(" connected!");
}

void connectToFavoriotMQTT() {
  Serial.print("Connecting to Favoriot MQTT ...");
  client.setCACert(rootCACertificate);
  mqtt.begin("mqtt.favoriot.com", 8883, client);
    
  // to create the client id can be a random configuration but its better 
  // to custom it for better organization
  // like right now im already customize it in the universal line (const char*)
  // String uniqueString = String(ssid) + "-" + String(random(1, 98)) + String(random(99, 999));
  // char ClientID[uniqueString.length() + 1];
  // uniqueString.toCharArray(ClientID, uniqueString.length() + 1);
  while (!mqtt.connect(ClientID, deviceAccessToken, deviceAccessToken)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" connected!");  
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  if (!apds.begin()){
    Serial.println("Not detecting the Hibiscus Sense sensor");
  }
  apds.enableProximity(true); 
  Serial.println();
  connectToWiFi();
  connectToFavoriotMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  if (!mqtt.connected()) {
    connectToFavoriotMQTT();
  }
  mqtt.loop();
  delay(10);  // <- fixes some issues with WiFi stability
  SendData();
  Serial.println();
  delay (250);
}

void SendData() {
  int proximity = apds.readProximity();
  Serial.println("Proximity: " + String(proximity));

  if(millis() - previousMillis > 15000){
    previousMillis = millis();
    // Prepare the JSON data to send
    String json = "{\"device_developer_id\":\"" + String(deviceDeveloperID) + "\",\"data\":{";
          json += "\"proximity\":\"" + String(proximity) + "\",";
          json += "}}";

    // Publishing to Favoriot
    Serial.println("\nSending data to Favoriot's Data Stream ...");
    Serial.println("Data to Publish: " + json);
    Serial.println("Publish to: " + String(deviceAccessToken) + String(publishTopic));
    // Check if the publish was successful
    bool success = mqtt.publish(String(deviceAccessToken) + String(publishTopic), json);
    if (success) {
        Serial.println("Data successfully sent to Favoriot!");
    } else {
        Serial.println("Failed to send data to Favoriot.");
    }
  }
}