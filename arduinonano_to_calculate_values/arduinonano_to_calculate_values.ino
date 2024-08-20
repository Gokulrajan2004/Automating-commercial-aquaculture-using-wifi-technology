#include <OneWire.h>
#include <DallasTemperature.h>

// Define the analog pins for the sensors
const int orpPin = A2;
const int TdsSensorPin = A1;
const int pH_Pin = A0; // Analog pin for pH sensor

// Data wire for the DS18B20 temperature sensor is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600); // Starts the serial communication
  sensors.begin(); // Start the DS18B20 sensor
}

void loop() {
  // Read sensors here
  float pH_Value = readPH();
  float temperature = readTemperature();
  float orp = readORP();
  float tdsValue = readTDS();
  float ecValue = calculateEC(tdsValue);

  // Create a comma-separated string of the sensor values
  String sensorData = String(pH_Value) + "," + String(temperature) + "," + String(orp) + "," + String(tdsValue) + "," + String(ecValue);

  // Send the string over serial to the ESP8266
  Serial.println(sensorData);

  // Wait a bit before reading the sensors again
  delay(5000);
}

float readPH() {
  int sensorValue = analogRead(pH_Pin);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  // Convert voltage to pH value. You might need to calibrate this function.
  float pH = (voltage - 2.5) * -5.7; // Example conversion, adjust according to your sensor and calibration
  return pH;
}

float readTemperature() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempCByIndex(0);
  return tempC;
}

float readORP() {
  int sensorValue = analogRead(orpPin);
  float voltage = sensorValue * (5.0 / 1023.0);
  float orpValue = (voltage - 2.5) * 1000; // Example conversion
  return orpValue;
}

float readTDS() {
  int sensorValue = analogRead(TdsSensorPin);
  float voltage = sensorValue * (5.0 / 1023.0);
  float tdsValue = voltage * 500; // Example conversion, adjust according to your sensor and calibration
  return tdsValue;
}

float calculateEC(float tds) {
  // Example calculation, adjust according to your needs
  float ecValue = tds / 500.0;
  return ecValue;
}
