#include "BLE.h"

#include "r4scfg.h"

BLEClient *pBLEClient = 0;
BLEAddress *pServerAddress = 0;
esp_ble_addr_type_t serverAddressType;

bool connected = false;
bool disconnected = false;
bool scanning = false;

char serverName[BLE_INPUT_BUFFSIZE];
uint8_t notifyData[BLE_INPUT_BUFFSIZE];
int8_t notifyDataLen = -1;

BLERemoteCharacteristic* pRemoteTXCharacteristic = 0;
BLERemoteCharacteristic* pRemoteRXCharacteristic = 0;
BLERemoteDescriptor* pRemoteRXDescriptor = 0;

// The remote service we wish to connect to.
BLEUUID serviceUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e");

// The characteristic of the remote service we are interested in.
BLEUUID    rxCharUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
BLEUUID    txCharUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
BLEUUID    rsDescrUUID((uint16_t) 0x2902);

void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {

  if (length > BLE_INPUT_BUFFSIZE)
    length = BLE_INPUT_BUFFSIZE;

  if (length) {
    memcpy(notifyData, pData, length);
  }

  notifyDataLen = length;
}

void MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
  log_d("BLE Advertised Device found : %s", advertisedDevice.toString().c_str());

  if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
    log_i("Found our device at address : %s", advertisedDevice.getAddress().toString().c_str());

    stopScanning();

    strcpy(serverName, advertisedDevice.getName().c_str());
    pServerAddress = new BLEAddress(*advertisedDevice.getAddress().getNative());
    serverAddressType = advertisedDevice.getAddressType();
  }
}


void MyBLEClientCallbacks::onConnect(BLEClient *pClient) {
  if (pClient->isConnected()) {
    log_i(" - Connected to server %s", pClient->getPeerAddress().toString().c_str());
  }
}

void MyBLEClientCallbacks::onDisconnect(BLEClient *pClient) {
  if (connected) {
    log_w(" - BLE Device connection lost ");
  } else {
    log_d(" - BLE Device disconnected ");
  }
  notifyDataLen = -2;
  disconnected = true;
}

bool checkServices() {
  if (!pBLEClient->isConnected())
    return false;

  BLERemoteService* pRemoteService = 0;
  // Obtain a reference to the service we are after in the remote BLE server.
  try {
    pRemoteService = pBLEClient->getService(serviceUUID);
    log_d(" - Found our %s", pRemoteService->toString().c_str());
  }
  catch (...) {
    log_e("Failed to find our service UUID : %s", serviceUUID.toString().c_str());
  }

  if (!pRemoteService)
    return false;

  pRemoteRXCharacteristic = pRemoteService->getCharacteristic(rxCharUUID);
  if (pRemoteRXCharacteristic == nullptr) {
    log_e("Failed to find our characteristic UUID: %s", rxCharUUID.toString().c_str());
    return false;
  }

  pRemoteRXCharacteristic->registerForNotify(notifyCallback);
  log_d(" - Found our RX characteristic with handle %x", pRemoteRXCharacteristic->getHandle());

  pRemoteTXCharacteristic = pRemoteService->getCharacteristic(txCharUUID);
  if (pRemoteTXCharacteristic == nullptr) {
    log_e("Failed to find our characteristic UUID: %s", txCharUUID.toString().c_str());
    return false;
  }
  log_d(" - Found our TX characteristic with handle %x", pRemoteTXCharacteristic->getHandle());

  pRemoteRXDescriptor = pRemoteRXCharacteristic->getDescriptor(rsDescrUUID);
  if (pRemoteRXDescriptor == nullptr) {
    log_e("Failed to find our descriptor UUID: %s", rsDescrUUID.toString().c_str());
    return false;
  }
  log_d(" - Found our descriptor with handle %x", pRemoteRXDescriptor->getHandle());

  uint16_t enNotify = 0x0001;
  try {
    pRemoteRXDescriptor->writeValue((uint8_t*)&enNotify, sizeof(enNotify));
    delay(10);
  } catch (...) {
    log_e("writeValue failed");
    return false;
  }

  return true;
}

bool connectToServer(BLEAddress *pAddress, esp_ble_addr_type_t type) {

  log_i("address : %s", pAddress->toString().c_str());

  if (!pBLEClient->connect(*pAddress, type)) {
    log_e("Failed to connect server with address : %s", pAddress->toString().c_str());
    return false;
  }

  if (!checkServices()) {
    if (pBLEClient->isConnected()) {
      disconnectFromServer();
    }
  }

  return pBLEClient->isConnected();
}

void disconnectFromServer() {
  connected = false;
  disconnected = false;
  pBLEClient->disconnect();
}

void scanDevices() {
  if (scanning)
    return;

  log_i("Starting device scan...");
  scanning = BLEDevice::getScan()->start(BLE_SCAN_DURATION, [](BLEScanResults) {
    scanning = false;
  }, false);
}

void stopScanning()
{
  BLEDevice::getScan()->stop();
  scanning = false;
}

void setupBLE() {

  BLEDevice::init("R4SGATE");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(100);

  pBLEClient = BLEDevice::createClient();
  pBLEClient->setClientCallbacks(new MyBLEClientCallbacks());
}

void freeBLEServer() {
  BLEDevice::getScan()->erase(*pServerAddress);
  delete pServerAddress;
  pServerAddress = 0;
}

bool loopBLE() {

  if (disconnected) {
    disconnectFromServer();
    return false;
  }

  if (!pServerAddress) {
    scanDevices();
    return false;
  }

  static uint8_t retries = bleConnectRetries;
  static uint8_t rescan_retries = bleConnectRetriesBeforeRescan;
  if (pServerAddress && !pBLEClient->isConnected()) {
    if (connectToServer(pServerAddress, serverAddressType)) {
      log_d("We are now connected to the BLE Server.");
      connected = true;
      retries = bleConnectRetries;
      rescan_retries = bleConnectRetriesBeforeRescan;
    } else {
      if (retries && (--retries == 0)) {
        log_e("Failed to connect to the server.");
        disconnected = true;
      }
      if (--rescan_retries == 0) {
        log_e("Failed to reconnect to the server. Scanning for new one...");
        freeBLEServer();
      }
      return false;
    }
  }

  return true;
}
