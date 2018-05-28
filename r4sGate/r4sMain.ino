
void setupWiFi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino R4S Gateway...");

  setupWiFi();

#ifndef R4SGATE_NO_MQTT
  setupMQTT();
#endif //R4SGATE_NO_MQTT

  setupWeb();
  setupBLE();
}

void loop() {

  if (disconnected) {
#ifndef R4SGATE_NO_MQTT
    mqttPublish(pServerAddress, "/status", "offline");
#endif //R4SGATE_NO_MQTT
    authorized = false;
    r4scounter = 0;
  }

#ifndef R4SGATE_NO_MQTT
  loopMQTT();
#endif //R4SGATE_NO_MQTT
  loopWeb();

  if (!loopBLE())
    return;

  if (pBLEClient->isConnected()) {

    if (!authorized) {
      if (authorize()) {
        uint16_t ver = r4sVersion();
        Serial.print("Server name : ");
        Serial.println(serverName);

        String verstr = String(ver >> 8) + "." + String(ver % 256);
        Serial.print("Firmware Version : ");
        Serial.println(verstr.c_str());

#ifndef R4SGATE_NO_MQTT
        mqttPublish(pServerAddress, "/name", serverName);
        mqttPublish(pServerAddress, "/version", verstr.c_str());
        mqttPublish(pServerAddress, "/status", "online");
        mqttSubscription(pServerAddress, true);
#endif //R4SGATE_NO_MQTT
      } else {
        disconnected = true;

#ifndef R4SGATE_NO_MQTT
        mqttPublish(pServerAddress, MQTT_ERROR_TOPIC, "AUTH_FAILED");
        mqttSubscription(pServerAddress, false);
#endif //R4SGATE_NO_MQTT

        freeBLEServer();
        Serial.println(" - Authorization failed");
        return;
      }
    }

#ifndef R4SGATE_NO_MQTT
    if (authorized) {
      if (r4sStatusUpdateTime) {
        static unsigned long stTime = 0;
        if (stTime + r4sStatusUpdateTime < millis()) {
          publishStatus(pServerAddress);
          stTime = millis();
        }
      }
    }
#endif //R4SGATE_NO_MQTT

  }
  delay(10);
}
