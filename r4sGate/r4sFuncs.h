#ifndef R4SFUNCS_H
#define R4SFUNCS_H

#include "r4scfg.h"

#include "RKM171S.h"
#include "MQTT.h"
#include "WebAPI.h"

void setupWiFi();

// MQTT funcitons
String mqttBaseTopic(BLEAddress* addr);

bool mqttPublish(BLEAddress* addr, const char* topic, const char* payload);
bool mqttSubscription(BLEAddress* addr, bool on);
void mqttConnected();
void mqttCommand(const char* topic, const char* payload);
bool publishStatus(BLEAddress* addr);


// R4S KETTLE Authorize
bool authorize(uint8_t retires = 5);

#endif
