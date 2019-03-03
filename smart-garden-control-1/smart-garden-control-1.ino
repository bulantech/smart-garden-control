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
String startValue = "1";
String stopValue = "2";
String maxTime = "3";
int relayState = 0;
int manual = 0;
int startValueInt = 0;
int stopValueInt = 0;
int maxTimeInt = 0;
//int relayOnCount = 0;
  
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

  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }
  if (!SPIFFS.exists("/startValue.txt")) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("create file");
    
    File startValueFile = SPIFFS.open("/startValue.txt", "w");
    startValueFile.println("1000");
    startValueFile.close();

    File stopValueFile = SPIFFS.open("/stopValue.txt", "w");
    stopValueFile.println("300");
    stopValueFile.close();

    File maxTimeFile = SPIFFS.open("/maxTime.txt", "w");
    maxTimeFile.println("60");
    maxTimeFile.close(); 
  } 

  File startValueFile = SPIFFS.open("/startValue.txt", "r");
  startValue = startValueFile.readStringUntil('\r');
  startValueFile.close();
  startValueInt = startValue.toInt();

  File stopValueFile = SPIFFS.open("/stopValue.txt", "r");
  stopValue = stopValueFile.readStringUntil('\r');
  stopValueFile.close();
  stopValueInt = stopValue.toInt();

  File maxTimeFile = SPIFFS.open("/maxTime.txt", "r");
  maxTime = maxTimeFile.readStringUntil('\r');
  maxTimeFile.close();
  maxTimeInt = (maxTime.toInt())*60;
    
  Serial.println("Read file: "+startValue+", "+stopValueFile+", "+maxTime+", ");
  
//  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/save", handleSave); 
  server.on("/disable-manual", handleDisableManual);
  
    //redirect all traffic to index.html
  server.onNotFound([]() {
//    handleRoot();
    if(!handleFileRead(server.uri())){
      String metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://" + myIP.toString() + "/index.html\" /></head><body><p>redirecting...</p></body>";
      server.send(200, "text/html", metaRefreshStr);
    }
  });
  
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
  sensorValue = analogRead(SENSOR);
//  sensorValue = 500; //test
  Serial.println("relayState, sensorValue, maxTimeInt, startValueInt, stopValueInt, manual = " + String(relayState) + ", " + String(sensorValue) + ", " + String(maxTimeInt) + ", " + String(startValueInt) + ", " + String(stopValueInt) + ", " + String(manual));
  if(manual) { 
    if(relayState) {
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    return; 
  }
  if(relayState) {
    digitalWrite(RELAY, HIGH); //on
    if( (--maxTimeInt <= 0) || (sensorValue <= stopValueInt) ) {
      maxTimeInt = 0;
      relayState = 0;
      digitalWrite(RELAY, LOW); //off      
    }
    uint8_t ledState = digitalRead(LED_BUILTIN);
    digitalWrite( LED_BUILTIN, !ledState );
  } 
  else {
    if(sensorValue >= startValueInt) {
      relayState = 1;
      maxTimeInt = (maxTime.toInt())*60;
      digitalWrite(RELAY, HIGH);
    }
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH); 
  }
   
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

void handleStatus() {
  String res = "";
  res += String(sensorValue)+",";
  res += String(relayState)+",";
  res += String(startValue)+",";
  res += String(stopValue)+",";
  res += String(maxTime)+","; 
  res += String(manual)+",";
  server.send(200, "text/html", res);
}

void handleOn() {
  String res = "ON";
  manual = 1;
  relayState = 1;
  maxTimeInt = (maxTime.toInt())*60;
  digitalWrite(RELAY, HIGH);
  server.send(200, "text/html", res);
}

void handleOff() {
  String res = "OFF";
  manual = 1;
  relayState = 0;
  digitalWrite(RELAY, LOW);
  server.send(200, "text/html", res);
}

void handleSave() {
  String res = "Save ok: ";    
  if (server.hasArg("plain")== false){ //Check if body received
    server.send(200, "text/plain", "Body not received");
    return; 
  }
  startValue = server.arg("startValue");
  stopValue = server.arg("stopValue");
  maxTime = server.arg("maxTime");

  File startValueFile = SPIFFS.open("/startValue.txt", "w");
  startValueFile.println(startValue);
  startValueFile.close();
  startValueInt = startValue.toInt();

  File stopValueFile = SPIFFS.open("/stopValue.txt", "w");
  stopValueFile.println(stopValue);
  stopValueFile.close();
  stopValueInt = stopValue.toInt();

  File maxTimeFile = SPIFFS.open("/maxTime.txt", "w");
  maxTimeFile.println(maxTime);
  maxTimeFile.close(); 
      
  res += startValue+",";
  res += stopValue+",";
  res += maxTime+",";
  server.send(200, "text/html", res);
}

void handleDisableManual() {
  String res = "Disable manual mode";
  manual = 0;
  server.send(200, "text/html", res);
}
