#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

#include "config.h"

WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;

void rootPage()
{
   char content[] = "ESP 32";
   Server.send(200, "text/plain", content);
}

void setup_wifi()
{
   WiFi.setHostname(HOSTNAME);
   Config.autoReconnect = true;
   Config.hostName = HOSTNAME;
   Config.apid = "ESP32-" + String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
   Config.psk = "3.1416";
   Portal.config(Config);

   Server.on("/", rootPage);
   if (Portal.begin())
   {
      Serial.println("[WIFI] Web server started: " + WiFi.localIP().toString());
   }
   else
   {
      Serial.println("[WIFI] Failed to start server");
   }
}

void wifi_loop()
{
   if (!WiFi.isConnected())
   {
      // Serial.println("[WIFI] handling client");
      Portal.handleClient();
   }
}