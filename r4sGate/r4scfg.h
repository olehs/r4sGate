#ifndef R4SCFG_H
#define R4SCFG_H

#include "Arduino.h"

//----------- WiFi Settings
static const char* ssid = "........";
static const char* password = "........";
static const char* dns_name = "r4sgate";

//----------- MQTT Settings
//#define MQTT_UPPERCASE_DEV_TOPIC
#define MQTT_BASE_TOPIC "r4s"
#define MQTT_CMND_TOPIC "/cmnd"
#define MQTT_STAT_TOPIC "/stat"
#define MQTT_ERROR_TOPIC "/error"

static const char* mqtt_server = "192.168.2.1";
static const uint16_t mqtt_port = 1883;
static const char* mqtt_client = "R4SClient";


//----------- WebAPI Settings
#define WEB_SERVER_PORT 80


//----------- BLE Settings
#define BLE_SCAN_DURATION 10

static uint8_t bleConnectRetries = 1; // Device connect retries before going offline
static uint8_t bleConnectRetriesBeforeRescan = 4; // Device reconnect retries before starting new scan


//----------- R4S Settings
//#define R4S_LOG_EXCHANGE
//#define R4S_G200S_SUPPORT

// Change this 8-byte Auth code randomly
static uint8_t r4sAuth[8] = { 0xb5, 0x4c, 0x75, 0xb1, 0xb4, 0xac, 0x88, 0xef };

static unsigned long r4sStatusUpdateTime = 5000;

#endif
