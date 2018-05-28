#define BLE_INPUT_BUFFSIZE 64

// The remote service we wish to connect to.
static BLEUUID serviceUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e");

// The characteristic of the remote service we are interested in.
static BLEUUID    rxCharUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
static BLEUUID    txCharUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
static BLEUUID    rsDescrUUID((uint16_t) 0x2902);

static BLEClient*  pBLEClient = 0;
static BLEAddress *pServerAddress = 0;

static boolean connected = false;
static boolean disconnected = false;
static char serverName[BLE_INPUT_BUFFSIZE];
static uint8_t notifyData[BLE_INPUT_BUFFSIZE];
static int8_t notifyDataLen = -1;

static BLERemoteCharacteristic* pRemoteTXCharacteristic = 0;
static BLERemoteCharacteristic* pRemoteRXCharacteristic = 0;
static BLERemoteDescriptor* pRemoteRXDescriptor = 0;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {

  notifyDataLen = length;
  if (notifyDataLen > BLE_INPUT_BUFFSIZE)
    notifyDataLen = BLE_INPUT_BUFFSIZE;

  if (length) {
    memcpy(&notifyData[0], pData, notifyDataLen);
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found : ");
      Serial.println(advertisedDevice.toString().c_str());

      if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
        Serial.print("Found our device at address : ");
        Serial.println(advertisedDevice.getAddress().toString().c_str());

        advertisedDevice.getScan()->stop();

        pServerAddress = new BLEAddress(advertisedDevice.getAddress());
        strcpy(&serverName[0], advertisedDevice.getName().c_str());
      }
    }
};

class MyBLEClientCallbacks: public BLEClientCallbacks {

    void onConnect(BLEClient *pClient) {
      if (pClient->isConnected()) {
        Serial.print(" - Connected to server ");
        Serial.println(pClient->getPeerAddress().toString().c_str());
      }
    }

    void onDisconnect(BLEClient *pClient) {
      if (connected) {
        Serial.println(" - BLE Device connection lost ");
      } else {
        Serial.println(" - BLE Device disconnected ");
      }
      notifyDataLen = -2;
      disconnected = true;
    }
};

bool checkServices() {
  if (!pBLEClient->isConnected())
    return false;

  BLERemoteService* pRemoteService = 0;
  // Obtain a reference to the service we are after in the remote BLE server.
  try {
    pRemoteService = pBLEClient->getService(serviceUUID);
    Serial.print(" - Found our ");
    Serial.println(pRemoteService->toString().c_str());
  }
  catch (BLEUuidNotFoundException e) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
  }

  if (!pRemoteService)
    return false;

  pRemoteRXCharacteristic = pRemoteService->getCharacteristic(rxCharUUID);
  if (pRemoteRXCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(rxCharUUID.toString().c_str());
    return false;
  }

  pRemoteRXCharacteristic->registerForNotify(notifyCallback);
  Serial.print(" - Found our RX characteristic with handle 0x");
  Serial.println(pRemoteRXCharacteristic->getHandle(), HEX);

  pRemoteTXCharacteristic = pRemoteService->getCharacteristic(txCharUUID);
  if (pRemoteTXCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(txCharUUID.toString().c_str());
    return false;
  }
  Serial.print(" - Found our TX characteristic with handle 0x");
  Serial.println(pRemoteTXCharacteristic->getHandle(), HEX);

  pRemoteRXDescriptor = pRemoteRXCharacteristic->getDescriptor(rsDescrUUID);
  if (pRemoteRXDescriptor == nullptr) {
    Serial.print("Failed to find our descriptor UUID: ");
    Serial.println(rsDescrUUID.toString().c_str());
    return false;
  }
  Serial.print(" - Found our descriptor with handle 0x");
  Serial.println(pRemoteRXDescriptor->getHandle(), HEX);

  uint16_t enNotify = 0x0001;
  pRemoteRXDescriptor->writeValue((uint8_t*)&enNotify, sizeof(enNotify));
  delay(10);

  return true;
}

bool connectToServer(BLEAddress pAddress) {

  Serial.print(" - Connecting to server with address : ");
  Serial.println(pAddress.toString().c_str());

  if (!pBLEClient->connect(pAddress)) {
    Serial.print("Failed to connect server with address : ");
    Serial.println(pAddress.toString().c_str());
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
  Serial.println("Starting device scan...");
  BLEDevice::getScan()->start(BLE_SCAN_DURATION);
}

void setupBLE() {

  BLEDevice::init("");
  pBLEClient = BLEDevice::createClient();
  pBLEClient->setClientCallbacks(new MyBLEClientCallbacks());

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void freeBLEServer() {
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
  if (pServerAddress && !pBLEClient->isConnected()) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
      retries = bleConnectRetries;

    } else {
      if (retries && (--retries == 0)) {
        Serial.println("Failed to connect to the server.");
        disconnected = true;
        //        freeBLEServer();
        return false;
      }
    }
  }

  return true;
}

