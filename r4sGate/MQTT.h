#ifndef R4SMQTT_H
#define R4SMQTT_H

#include "WiFiClient.h"
#include "PubSubClient.h"

#include "r4scfg.h"

extern WiFiClient espClient;
extern PubSubClient mqttClient;

void mqttConnected();
void mqttCommand(const char* topic, const char* payload);

static void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();

void setupMQTT();
void loopMQTT();

#endif
