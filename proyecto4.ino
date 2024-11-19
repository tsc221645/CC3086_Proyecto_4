#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// Wi-Fi credentials
const char* ssid = "Sofia's Galaxy A53 5G";
const char* password = "sofi1008";

// Google Apps Script URL
const char* host = "script.google.com";
const char* path = "/macros/s/AKfycbzjuSOJ9uahfLsr1NETa7nAOz151cwIjTcMq3MxY_3YYR3ibR_EDZK8EZ_L9-91OGkddQ/exec";

// Sensor pins
const int lm35Pin = A0;  // Analog pin for LM35D (Temperature sensor)
const int mq135Pin = D0; // Digital pin for MQ135 (Air quality sensor)
const int flamePin = D1; // Digital pin for Flame sensor

// Function to encode URL parameters
String urlEncode(String str) {
  String encodedString = "";
  char c;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encodedString += c;
    } else {
      encodedString += "%" + String(c, HEX);
    }
  }
  return encodedString;
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  pinMode(mq135Pin, INPUT);
  pinMode(flamePin, INPUT);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Read sensor data
    float temperature = analogRead(lm35Pin); // Convert analog value to temperature
    int airQuality = digitalRead(mq135Pin); // Read digital value from MQ135 (0 or 1)
    int flameDetected = digitalRead(flamePin); // Read flame detection (1 or 0)

    // Log sensor values
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Air Quality: ");
    Serial.println(airQuality);
    Serial.print("Flame Detected: ");
    Serial.println(flameDetected);

    // Construct query string
    String query = String(path) + 
                   "?temperature=" + urlEncode(String(temperature, 2)) + 
                   "&airQuality=" + urlEncode(String(airQuality)) + 
                   "&flameDetected=" + urlEncode(String(flameDetected));

    Serial.println("Requesting URL: https://" + String(host) + query);

    // Use WiFiClientSecure for HTTPS
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate validation for simplicity; consider using a certificate for production

    if (client.connect(host, 443)) {
      String request = "GET " + query + " HTTP/1.1\r\n" +
                       "Host: " + String(host) + "\r\n" +
                       "Connection: close\r\n\r\n";
      client.print(request);
      Serial.println("Request sent");

      // Read response
      while (client.connected() || client.available()) {
        if (client.available()) {
          String line = client.readStringUntil('\n');
          Serial.println(line);  // Print the response for debugging
        }
      }
      client.stop();
      Serial.println("Disconnected from server");
    } else {
      Serial.println("Connection failed");
    }
  }

  delay(5000); // Delay before the next loop
}