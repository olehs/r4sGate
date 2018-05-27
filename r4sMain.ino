
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
#ifndef MQTT_DISABLED
  setupMQTT();
#endif //MQTT_DISABLED
  setupWeb();
  setupBLE();
}

void loop() {

  if (disconnected) {
#ifndef MQTT_DISABLED
    mqttPublish(pServerAddress, "/Status", "Offline");
#endif //MQTT_DISABLED
    authorized = false;
    r4scounter = 0;
  }

#ifndef MQTT_DISABLED
  loopMQTT();
#endif //MQTT_DISABLED
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

#ifndef MQTT_DISABLED
        mqttPublish(pServerAddress, "/Name", serverName);
        mqttPublish(pServerAddress, "/Version", verstr.c_str());
        mqttPublish(pServerAddress, "/Status", "Online");
        mqttSubscription(pServerAddress, true);
#endif //MQTT_DISABLED
      } else {
        disconnected = true;

#ifndef MQTT_DISABLED
        mqttPublish(pServerAddress, "/Status", "AuthFailed");
        mqttSubscription(pServerAddress, false);
#endif //MQTT_DISABLED

        freeBLEServer();
        Serial.println(" - Authorization failed");
        return;
      }
    }

#ifndef MQTT_DISABLED
    if (authorized) {
      if (r4sStatusUpdateTime) {
        static unsigned long stTime = 0;
        if (stTime + r4sStatusUpdateTime < millis()) {
          publishStatus(pServerAddress);
          stTime = millis();
        }
      }
    }
#endif //MQTT_DISABLED

  }
  delay(10);
}
