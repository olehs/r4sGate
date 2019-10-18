#ifndef R4SBLE_H
#define R4SBLE_H

#include "BLEDevice.h"
#include "Arduino.h"

#define BLE_INPUT_BUFFSIZE 64

extern BLEClient *pBLEClient;
extern BLEAddress *pServerAddress;
extern esp_ble_addr_type_t serverAddressType;

extern bool connected;
extern bool disconnected;
extern bool scanning;

extern char serverName[BLE_INPUT_BUFFSIZE];
extern uint8_t notifyData[BLE_INPUT_BUFFSIZE];
extern int8_t notifyDataLen;

extern BLERemoteCharacteristic* pRemoteTXCharacteristic;
extern BLERemoteCharacteristic* pRemoteRXCharacteristic;
extern BLERemoteDescriptor* pRemoteRXDescriptor;

void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice);
};

class MyBLEClientCallbacks: public BLEClientCallbacks
{
    void onConnect(BLEClient *pClient);
    void onDisconnect(BLEClient *pClient);
};

bool checkServices();
bool connectToServer(BLEAddress *pAddress, esp_ble_addr_type_t type);
void disconnectFromServer();
void scanDevices();
void stopScanning();
void setupBLE();
void freeBLEServer();
bool loopBLE();

#endif
