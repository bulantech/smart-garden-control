#include <Ticker.h>

#define LED_BUILTIN D4
#define RELAY   D8
#define SENSOR  A0

int sensorValue = 0;

Ticker interval1Sec;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  Serial.println();
  Serial.println("Start..");
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY, OUTPUT);

  interval1Sec.attach(1, readSensor);
}

// the loop function runs over and over again forever
void loop() {
//  sensorValue = analogRead(SENSOR);
//  Serial.println("sensorValue = " + String(sensorValue) );
//  
//  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//  digitalWrite(RELAY, HIGH);
//  Serial.println("Relay close");
//  delay(1000);                       // wait for a second
//  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//  digitalWrite(RELAY, LOW);
//  Serial.println("Relay open");
//  delay(1000);                       // wait for a second
}

void readSensor() {
  digitalWrite(LED_BUILTIN, LOW);
  sensorValue = analogRead(SENSOR);
  Serial.println("sensorValue = " + String(sensorValue) );
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
 
}
