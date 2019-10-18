#include "r4sFuncs.h"

void setup() {
  Serial.begin(115200);
  log_i("Starting Arduino R4S Gateway...");

  setupWiFi();
  setupMQTT();
  setupWeb();
  setupBLE();
}

void loop() {
  if (disconnected) {
    mqttPublish(pServerAddress, "/status", "offline");
    authorized = false;
  }

  loopMQTT();
  loopWeb();

  if (!loopBLE())
    return;

  if (pBLEClient->isConnected()) {

    if (!authorized) {
      if (authorize()) {
        uint16_t ver = r4sVersion();
        log_i("Server name : %s", serverName);

        String verstr = String(ver >> 8) + "." + String(ver % 256);
        log_i("Firmware Version : %s", verstr.c_str());

        mqttPublish(pServerAddress, "/name", serverName);
        mqttPublish(pServerAddress, "/version", verstr.c_str());
        mqttPublish(pServerAddress, "/rssi", String(pBLEClient->getRssi()).c_str());
        mqttPublish(pServerAddress, "/status", "online");
        mqttSubscription(pServerAddress, true);
      } else {
        disconnected = true;

        mqttPublish(pServerAddress, MQTT_ERROR_TOPIC, "AUTH_FAILED");
        mqttSubscription(pServerAddress, false);

        freeBLEServer();
        log_e(" - Authorization failed");
        return;
      }
    }

    if (authorized) {
      if (r4sStatusUpdateTime) {
        static unsigned long stTime = 0;
        if (stTime + r4sStatusUpdateTime < millis()) {
          publishStatus(pServerAddress);
          stTime = millis();
        }
      }
    }

  }
  delay(10);
}
