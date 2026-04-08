#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <WebServer.h>
#include <WiFi.h>
#include <IRutils.h>
#include <HTTPClient.h>

const char* ssid = "WIFINAME";
const char* password = "WIFIPASSWORD";
const char* serverUrl = "http://0.0.0.0:3000";

const uint16_t RECV_PIN = 4;
const uint16_t SEND_PIN = 5;


IRsend irsend(SEND_PIN);
IRrecv irrecv(RECV_PIN);
decode_results results;

bool sendIr = false;
bool receiveIr = false;
bool enableWebServer = false;
bool sendRequestToServerUrl = false; //I have a internal api setupped to automatically save the ip for requesting IR trasmit using a static ip

if (enableWebServer){
  WebServer server(80);
}

#define CMD_POWER 0x00FF609F
#define CMD_BRIGHTNESS_UP 0x00FF08F7
#define CMD_BRIGHTNESS_DOWN 0x00FFC03F
#define CMD_WHITE 0x00FF807F
#define CMD_COLORS 0x00FF9867
#define CMD_FIRE 0x00FFB04F
#define CMD_FIRE 0x00
#define CMD_ALLCOLORS 0x00FFE817
#define CMD_WAVE 0x00FFB24D

void rainbow(){

  irsend.sendNEC(CMD_POWER, 32);
  delay(200);
  irsend.sendNEC(CMD_ALLCOLORS, 32);
  delay(200);
  irsend.sendNEC(CMD_WAVE, 32);

}

void handleSend() {
  if (!server.hasArg("code")) {
    server.send(400, "text/plain", "Missing 'code' parameter");
    return;
  }
  String codeStr = server.arg("code");

  uint32_t code = strtoul(codeStr.c_str(), NULL, 16);
  for (int i = 0; i < 3; i++) {
    irsend.sendNEC(code, 32);
    delay(40);
  }

  server.send(200, "text/plain", "Sent: 0x" + codeStr);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  if (!enableWebServer) return;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());
  Serial.println("started");
  irrecv.enableIRIn();
  irsend.begin();

  server.on("/send", handleSend);
  server.begin();
  Serial.println("setup finished");
  if (WiFi.status() == WL_CONNECTED && sendRequestToServerUrl) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json"); 

    int httpResponseCode = http.POST("{}");
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from server: " + response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }
    http.end();
  }
}

void loop() {
  if (enableWebServer){
    server.handleClient();
  }
  if (receiveIr){
    if (irrecv.decode(&results)) {
      Serial.println(resultToSourceCode(&results));
      Serial.println(resultToHumanReadableBasic(&results));
      delay(10);
      irrecv.resume();
    }
  }
  if (sendIr){
    irsend.sendNEC(CMD_POWER, 32);
    delay(200);
  }
  
}
