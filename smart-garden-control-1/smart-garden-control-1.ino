#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#define LED_BUILTIN D4
#define RELAY   D8
#define SENSOR  A0

#ifndef APSSID
#define APSSID "SmartGarden"
#define APPSK  "thereisnospoon"
#endif

const byte DNS_PORT = 53;

int sensorValue = 0;
/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

Ticker interval1Sec;
ESP8266WebServer server(80);
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  Serial.println();
  Serial.println("Start..");
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY, OUTPUT);

  interval1Sec.attach(1, readSensor);

//  WiFi.softAP(ssid, password);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);
  
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //redirect all traffic to index.html
  server.onNotFound([]() {
    handleRoot();
  });
  
  server.on("/", handleRoot);
  server.begin();
  
  Serial.println("HTTP server started");  
}

// the loop function runs over and over again forever
void loop() {
  server.handleClient();
  dnsServer.processNextRequest();
  
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

void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}
