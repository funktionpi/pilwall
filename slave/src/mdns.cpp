#ifdef ESP32
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

#include <FastLED.h>

#include "log.h"
#include "config.h"

char matrix_width[4];
char matrix_height[4];
char panel_width[4];
char panel_height[4];
char tile_width[4];
char tile_height[4];
char bytes_count[2];

void setup_txt_entry(const char *service, const char *protocol)
{
   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "pixels_bytes", bytes_count));
   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "colors_order", "RGB"));

   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "matrix_width", matrix_width));
   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "matrix_height", matrix_height));

   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "panel_width", panel_width));
   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "panel_height", panel_height));

   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "tiles_horizontal", tile_width));
   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "tiles_vertical", tile_height));

   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "panel_layout", "ColumnMajorAlternatingLayout"));
   ESP_ERROR_CHECK(mdns_service_txt_item_set(service, protocol, "tile_layout", "RowMajorLayout"));
}

void setup_mdns()
{
   LOGLN("[mDNS] initializing mDNS service")
   ESP_ERROR_CHECK(mdns_init());

   sprintf(matrix_width, "%d", MATRIX_WIDTH);
   sprintf(matrix_height, "%d", MATRIX_HEIGHT);
   sprintf(panel_width, "%d", MATRIX_TILE_WIDTH);
   sprintf(panel_height, "%d", MATRIX_TILE_HEIGHT);
   sprintf(tile_width, "%d", MATRIX_TILE_H);
   sprintf(tile_height, "%d", MATRIX_TILE_V);
   sprintf(bytes_count, "%d", sizeof(CRGB));

   //set hostname
   ESP_ERROR_CHECK(mdns_hostname_set(BONJOUR_NAME));
   ESP_ERROR_CHECK(mdns_instance_name_set(HOSTNAME));

   ESP_ERROR_CHECK(mdns_service_add(nullptr, "_http", "_tcp", 80, nullptr, 0));
   ESP_ERROR_CHECK(mdns_service_txt_item_set("_http", "_tcp", "path", "/_ac"));
   ESP_ERROR_CHECK(mdns_service_instance_name_set("_http", "_tcp", "LEDsWall: Wifi configuration server"));

#if ENABLE_PROTO
   ESP_ERROR_CHECK(mdns_service_add(nullptr, "piproto", "_udp", PROTO_PORT, nullptr, 0));
   ESP_ERROR_CHECK(mdns_service_instance_name_set("piproto", "_udp", "LEDsWall: Pi Protocol service"));
   setup_txt_entry("piproto", "_udp");
#endif

#if ENABLE_OCP
   ESP_ERROR_CHECK(mdns_service_add(nullptr, "ocp", "_udp", OPC_PORT, nullptr, 0));
   ESP_ERROR_CHECK(mdns_service_instance_name_set("ocp", "_udp", "LEDsWall: Open Pixel Controller service"));
   setup_txt_entry("ocp", "_udp");
#endif

#if ENABLE_TPM2
   ESP_ERROR_CHECK(mdns_service_add(nullptr, "tpm2", "_udp", TPM2_IN_PORT, nullptr, 0));
   ESP_ERROR_CHECK(mdns_service_instance_name_set("tpm2", "_udp", "LEDsWall: TPM2.Net service"));
   setup_txt_entry("tpm2", "_udp");
#endif

#if ENABLE_ARTNET
   ESP_ERROR_CHECK(mdns_service_add(nullptr, "artnet", "_udp", ARTNET_PORT, nullptr, 0));
   ESP_ERROR_CHECK(mdns_service_instance_name_set("artnet", "_udp", "LEDsWall: ArtNet service"));
   setup_txt_entry("artnet", "_udp");
#endif

#if ENABLE_E131
   ESP_ERROR_CHECK(mdns_service_add(nullptr, "e131", "_udp", PROTO_PORT, nullptr, 0));
   ESP_ERROR_CHECK(mdns_service_instance_name_set("e131", "_udp", "LEDsWall: sACN e131 service"));
   setup_txt_entry("e131", "_udp");
#endif

   LOGLN("[mDNS] mDNS setup done!");
}