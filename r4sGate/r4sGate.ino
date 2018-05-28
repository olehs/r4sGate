#define R4SGATE_NO_MQTT

#include "WiFi.h"
#include "WiFiClient.h"
#include "ESPmDNS.h"
#include "WebServer.h"

#ifndef R4SGATE_NO_MQTT
#include "PubSubClient.h"
#endif //R4SGATE_NO_MQTT

#include "BLEDevice.h"
#include "m171s.h"

//----------- WiFi Settings
const char* ssid = "........";
const char* password = "........";
const char* dns_name = "r4sgate";

//----------- MQTT Settings
//#define MQTT_UPPERCASE_DEV_TOPIC

const char* mqtt_server = "192.168.1.100";
const uint16_t mqtt_port = 1883;
const char* mqtt_client = "R4SClient";


//----------- WebAPI Settings
#define WEB_SERVER_PORT 80


//----------- BLE Settings
#define BLE_SCAN_DURATION 10

// Device connect retries count before offline
static uint8_t bleConnectRetries = 1;


//----------- R4S Settings
//#define R4S_LOG_EXCHANGE

// Change this 8-byte Auth code randomly
static uint8_t r4sAuth[8] = { 0xb5, 0x4c, 0x75, 0xb1, 0xb4, 0xac, 0x88, 0xef };

static unsigned long r4sStatusUpdateTime = 5000;
