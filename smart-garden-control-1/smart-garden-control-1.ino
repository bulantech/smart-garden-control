#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>

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
IPAddress myIP;

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
  myIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);

  SPIFFS.begin();
  
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //redirect all traffic to index.html
  server.onNotFound([]() {
//    handleRoot();
    if(!handleFileRead(server.uri())){
      String metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://" + myIP.toString() + "/index.html\" /></head><body><p>redirecting...</p></body>";
      server.send(200, "text/html", metaRefreshStr);
    }
  });
  
//  server.on("/", handleRoot);
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

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//Given a file path, look for it in the SPIFFS file storage. Returns true if found, returns false if not found.
bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
