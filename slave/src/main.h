#ifndef __LED_SLAVE_H__
#define __LED_SLAVE_H__

void setup_wifi();
void setup_mdns();
void setup_websocket();
void setup_tcp();
void setup_udp();
void setup_raw();
void setup_artnet();

void artnet_tick();
void wifi_tick();
void led_tick();

#endif