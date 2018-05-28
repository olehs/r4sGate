
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

#ifndef R4SGATE_NO_OTA
// Port defaults to 3232
// ArduinoOTA.setPort(3232);

// Hostname defaults to esp3232-[MAC]
// ArduinoOTA.setHostname("myesp32");

// No authentication by default
// ArduinoOTA.setPassword("admin");

// Password can be set with it's md5 value as well
// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
void setupOTA() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)         Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)     Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
#endif //R4SGATE_NO_OTA

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino R4S Gateway...");

  setupWiFi();

#ifndef R4SGATE_NO_OTA
  setupOTA();
#endif //R4SGATE_NO_OTA

#ifndef R4SGATE_NO_MQTT
  setupMQTT();
#endif //R4SGATE_NO_MQTT

  setupWeb();
  setupBLE();
}

void loop() {
#ifndef R4SGATE_NO_OTA
  ArduinoOTA.handle();
  yield();
#endif //R4SGATE_NO_OTA

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
        mqttPublish(pServerAddress, "/rssi", String(pBLEClient->getRssi()).c_str());
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
