#ifndef __LED_SLAVE_H__
#define __LED_SLAVE_H__

void setup_wifi();
void setup_mdns();
void setup_websocket();
void setup_udp();
void setup_artnet();

void artnet_loop();
void wifi_loop();


#endif