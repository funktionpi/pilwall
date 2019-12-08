#pragma once

void setup_wifi();
void setup_mdns();
void setup_proto();
void setup_raw();
void setup_artnet();
void setup_opc();
void setup_e131();
void setup_tpm2();

void tick_artnet();
void tick_wifi();
void tick_led();
void tick_proto();
void tick_opc();
void tick_e131();
void tick_tpm2();