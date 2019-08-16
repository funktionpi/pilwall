#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>


#include "led.h"

const char* HOSTNAME = "esp32";
#define TCP_PORT 7050

WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;
// AsyncServer* Async;

void rootPage() {
  char content[] = "ESP 32";
  Server.send(200, "text/plain", content);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting esp32 setup");

  WiFi.setHostname(HOSTNAME);
  Config.autoReconnect = true;
  Config.hostName = HOSTNAME;
  Config.apid = "ESP32-" + String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
  Config.psk = "3.1416";
  Portal.config(Config);

  Server.on("/", rootPage);
  if (Portal.begin()) {
    Serial.println("Web server started: " + WiFi.localIP().toString());
  } else {
    Serial.println("Failed to start server");
  }

  // Serial.println("setting up async webserver")
	// Async = new AsyncServer(TCP_PORT); // start listening on tcp port 7050
	// server->onClient(&handleNewClient, server);
	// server->begin();

  led_setup();

  Serial.println("Setup done");
}

void loop() {
  Portal.handleClient();
  led_loop();
}

