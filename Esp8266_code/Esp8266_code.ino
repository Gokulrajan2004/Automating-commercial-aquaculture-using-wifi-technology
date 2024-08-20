#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "health"
#define WIFI_PASSWORD "1234567890"
#define RELAY_PIN 5

// Insert Firebase project API Key
#define API_KEY "AIzaSyD6Wi9evdOC6k7XsEGetwTyQCKfqzh5SuY"

// Insert RTDB URL
#define DATABASE_URL "https://batman-49a37-default-rtdb.europe-west1.firebasedatabase.app/" 

// Define Firebase Data object
FirebaseData firebaseData;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SoftwareSerial espSerial(4, 0); // RX, TX

float sensorValues[5]; // Array to hold pH, Temperature, ORP, TDS, EC

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2);
  display.clearDisplay();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Assign the API key (required)
  config.api_key = API_KEY;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  // Sign up
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void loop() {
  String path = "/relay1",a;
   if (Firebase.RTDB.getString(&firebaseData, path)) {
    if (firebaseData.dataType() == "string") {
      String data = firebaseData.stringData();
      Serial.print("Data: ");
      Serial.println(data);
      a=data;
      // Control the relay based on the data
      if (data == "0") {
        digitalWrite(RELAY_PIN, HIGH);  // Turn on the relay
      } else if (data == "1") {
        digitalWrite(RELAY_PIN, LOW);   // Turn off the relay
      } else {
        Serial.println("Invalid data received");
      }
    } else {
      Serial.println("No data found or data type is not a string");
    }
  } else {
    Serial.print("Error getting data: ");
    Serial.println(firebaseData.errorReason());
  }
  if (a == "0") {
        digitalWrite(RELAY_PIN, HIGH);  // Turn on the relay
      } else if (a == "1") {
        digitalWrite(RELAY_PIN, LOW);   // Turn off the relay
      } else {
        Serial.println("Invalid data received");
      }

  if (espSerial.available()) {
    String data = espSerial.readStringUntil('\n'); // Read the incoming data until newline
    Serial.println(data); // Debug print

    // Split and parse the incoming data
    int index = 0;
    char strBuf[100];
    data.toCharArray(strBuf, sizeof(strBuf));
    char* token = strtok(strBuf, ",");

    while (token != NULL && index < 5) {
      sensorValues[index++] = atof(token); // Convert string to float and store in array
      token = strtok(NULL, ",");
    }

    // Now, sensorValues array holds pH, Temperature, ORP, TDS, EC in that order
    // You can display these values or process them as needed
    displaySensorValues();
  }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setInt(&fbdo, "Values/Temperature", sensorValues[1])){
      Serial.println("Temperature PASSED");
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "Values/TDS", sensorValues[3])){
      Serial.println("TDS value PASSED");
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "Values/EC", sensorValues[4])){
      Serial.println("EC value PASSED");
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "Values/ORP", sensorValues[2])){
      Serial.println("ORP PASSED");
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "Values/Ph", sensorValues[0])){
      Serial.println("Ph PASSED");
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}

void displaySensorValues() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Display each sensor value
  display.println("pH: " + String(sensorValues[0]));
  display.println("Temp: " + String(sensorValues[1]) + " C");
  display.println("ORP: " + String(sensorValues[2]) + " mV");
  display.println("TDS: " + String(sensorValues[3]) + " ppm");
  display.println("EC: " + String(sensorValues[4]));

  display.display(); // Update the display with new text
}
