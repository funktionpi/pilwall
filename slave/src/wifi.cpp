#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

#include "config.h"
#include "log.h"

WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;

void rootPage()
{
   char content[] = "<meta http-equiv=\"refresh\" content=\"0; url=/_ac\">";
   Server.send(200, "text/plain", content);
}

void setup_wifi()
{
   WiFi.setHostname(HOSTNAME);
   Config.autoReconnect = true;
   Config.hostName = HOSTNAME;
   Config.apid = "ESP32-" + String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
   Config.psk = "3.141592";
   Config.apip = IPAddress(192,168,69,1);
   Config.gateway = IPAddress(192,168,69,1);
   Portal.config(Config);

   Server.on("/", rootPage);
   if (Portal.begin())
   {
      LOGLN("[WIFI] Web server started: " + WiFi.localIP().toString());
   }
   else
   {
      LOGLN("[WIFI] Failed to start server");
   }
}

void wifi_tick()
{
   Portal.handleClient();
}