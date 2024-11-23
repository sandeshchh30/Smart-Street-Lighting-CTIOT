#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

const char* ssid = "Your_SSID";          // Replace with your Wi-Fi SSID
const char* password = "Your_PASSWORD";  // Replace with your Wi-Fi Password
const char* server = "api.thingspeak.com";
WiFiClient client;

// ThingSpeak channel details
unsigned long channelID = YOUR_CHANNEL_ID;             // Replace with your channel ID
const char* writeAPIKey = "Your_Write_API_Key";  // Replace with your Write API Key

void setup() {
  Serial.begin(115200);  // For debugging
  WiFi.begin(ssid, password);

  // Connect to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  if (Serial.available()) {
    // Read data from Arduino
    String sensorData = Serial.readString();
    Serial.println("Received data: " + sensorData);
    
    // Parse and send data to ThinkSpeak
    int field1 = extractValue(sensorData, "LDR");
    int field2 = extractValue(sensorData, "PIR");
    int field3 = extractValue(sensorData, "Ultrasonic");

    if (client.connect(server, 80)) {
      String postStr = String("api_key=") + writeAPIKey + "&field1=" + String(field1) + "&field2=" + String(field2) + "&field3=" + String(field3) + "\r\n\r\n";
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
    }
    client.stop();
    delay(20000);  // Thingspeak update interval
  }
}

int extractValue(String data, String key) {
  int index = data.indexOf(key + ":");
  if (index == -1) return 0;
  int start = index + key.length() + 1;
  int end = data.indexOf(",", start);
  if (end == -1) end = data.length();
  return data.substring(start, end).toInt();
}
