#include <SoftwareSerial.h>

// Define pins for sensors
#define LDR_PIN A0
#define PIR_PIN 2
#define TRIG_PIN 3
#define ECHO_PIN 4

// Define pins for LEDs
#define WHITE_LED1 5
#define WHITE_LED2 6
#define WHITE_LED3 7
#define RGB_RED 8    // New RGB light for fault detection
#define RGB_GREEN 9
#define RGB_BLUE 10

// Communication with ESP8266
SoftwareSerial espSerial(18, 19); // RX1, TX1

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  espSerial.begin(115200);

  // Set sensor pins
  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Set LED pins
  pinMode(WHITE_LED1, OUTPUT);
  pinMode(WHITE_LED2, OUTPUT);
  pinMode(WHITE_LED3, OUTPUT);
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
}

void loop() {
  // Variables for fault detection
  bool ldrFault = false, pirFault = false, ultrasonicFault = false;

  // ** LDR Sensor **
  int ldrValue = analogRead(LDR_PIN);
  bool isDark = ldrValue < 500; // Adjust threshold if needed
  if (ldrValue < 0 || ldrValue > 1023) ldrFault = true; // Detect out-of-range

  // ** PIR Motion Sensor **
  bool motionDetected = digitalRead(PIR_PIN);
  static unsigned long pirLastSignalTime = millis();
  if (motionDetected) pirLastSignalTime = millis();
  if (millis() - pirLastSignalTime > 30000) pirFault = true; // No signal for 10 seconds

  // ** Ultrasonic Sensor **
  long duration, distance;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  bool isOccupied = (distance < 100); // Adjust range as needed
  if (distance <= 0 || distance > 400) ultrasonicFault = true; // Out of range

  // ** Control LEDs based on conditions **
  if (isDark) {
    digitalWrite(WHITE_LED1, HIGH); // Base lighting
    if (motionDetected || isOccupied) {
      digitalWrite(WHITE_LED2, HIGH);
      digitalWrite(WHITE_LED3, HIGH);
    } else {
      digitalWrite(WHITE_LED2, LOW);
      digitalWrite(WHITE_LED3, LOW);
    }
  } else {
    digitalWrite(WHITE_LED1, LOW);
    digitalWrite(WHITE_LED2, LOW);
    digitalWrite(WHITE_LED3, LOW);
  }

  // ** Fault Detection and RGB Light Indication **
  if (ldrFault || pirFault || ultrasonicFault) {
    if (ldrFault) {
      setRGBColor(255, 0, 0); // RED: LDR Fault
    } else if (pirFault) {
      setRGBColor(0, 255, 0); // GREEN: PIR Fault
    } else if (ultrasonicFault) {
      setRGBColor(0, 0, 255); // BLUE: Ultrasonic Fault
    }
  } else {
    setRGBColor(0, 0, 0); // Turn off RGB if no faults
  }

  // ** Send data to ESP8266 **
  String data = "LDR:" + String(ldrValue) + ",  PIR:" + String(motionDetected) + 
                ",  Ultrasonic:" + String(distance);

  if(ldrFault || pirFault || ultrasonicFault){
    if (ldrFault) {
      data += ",   LDR_Fault : " + String(ldrFault);
    } else if (pirFault) {
      data += ",   PIR_Fault : " + String(pirFault);
    } else if (ultrasonicFault) {
      data += ",   Ultrasonic_Fault : " + String(ultrasonicFault);
    }
  }
  espSerial.println(data);

  // Print simplified output to Serial Monitor
  Serial.println(data);

  delay(1000); // Wait 1 second
}

void setRGBColor(int red, int green, int blue) {
  analogWrite(RGB_RED, red);
  analogWrite(RGB_GREEN, green);
  analogWrite(RGB_BLUE, blue);
}
