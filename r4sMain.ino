
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
  setupMQTT();
  setupWeb();
  setupBLE();
}

void loop() {

  if (disconnected) {
    mqttPublish(pServerAddress, "/Status", "Offline");
    authorized = false;
    r4scounter = 0;
  }

  loopMQTT();
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

        mqttPublish(pServerAddress, "/Name", serverName);
        mqttPublish(pServerAddress, "/Version", verstr.c_str());
        mqttPublish(pServerAddress, "/Status", "Online");
        mqttSubscription(pServerAddress, true);
      } else {
        disconnected = true;

        mqttPublish(pServerAddress, "/Status", "AuthFailed");
        mqttSubscription(pServerAddress, false);

        freeBLEServer();
        Serial.println(" - Authorization failed");
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
