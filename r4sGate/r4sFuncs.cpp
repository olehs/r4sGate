#include "r4sFuncs.h"

#include "WiFi.h"
#include "ESPmDNS.h"

void webHandleDefault() {
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);

  String sTopic = webServer.uri();
  log_i("Web request : %s", sTopic.c_str());

  if (sTopic.equals("/")) {
    webServer.sendHeader("Location", String("http://") + WiFi.localIP().toString() + String("/" MQTT_BASE_TOPIC), true);
    webServer.send(301);
    webServer.sendContent("");
    return;
  }

  if (!sTopic.startsWith("/" MQTT_BASE_TOPIC)) {
    webServer.send(404);
    webServer.sendContent("");
    return;
  }

  if (sTopic.equals("/" MQTT_BASE_TOPIC)) {
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send(200, "application/json", "");

    webServer.sendContent("{\"monitoring\":");
    webServer.sendContent(String((long)(r4sStatusUpdateTime / 1000)));

    if (pServerAddress) {
      webServer.sendContent(",");
      webServer.sendContent("\"");
      String sAddr = String(pServerAddress->toString().c_str());
      sAddr.replace(":", "");
#ifdef MQTT_UPPERCASE_DEV_TOPIC
      sAddr.toUpperCase();
#endif
      webServer.sendContent(sAddr);
      webServer.sendContent("\":{\"name\":\"");
      webServer.sendContent(serverName);
      webServer.sendContent("\",\"status\":\"");
      if (pBLEClient->isConnected()) {
        webServer.sendContent("online\",\"rssi\":");
        webServer.sendContent(String(pBLEClient->getRssi()).c_str());
        webServer.sendContent(",\"state\":");
        webServer.sendContent(getStatusJson(pServerAddress));
      } else {
        webServer.sendContent("offline\"");
      }
      webServer.sendContent("}");
    }
    webServer.sendContent("}");
    webServer.sendContent("");
    return;
  }

  if (sTopic.equals("/" MQTT_BASE_TOPIC "/monitoring")) {
    String ival = webServer.arg("interval");
    if (ival.length()) {
      r4sStatusUpdateTime = ival.toInt() * 1000;
    }
    webServer.send(200, "application/json", "");
    webServer.sendContent("{\"result\":");
    webServer.sendContent(String((long)(r4sStatusUpdateTime / 1000)));
    webServer.sendContent("}");
    webServer.sendContent("");
    return;
  }

  if (!pServerAddress || !pBLEClient->isConnected()) {
    webServer.send(200, "application/json", "");
    webServer.sendContent("{\"error\":\"NOT_CONNECTED\"}");
    webServer.sendContent("");
    return;
  }

  String base = String("/") + mqttBaseTopic(pServerAddress) + "/";
  if (!sTopic.startsWith(base)) {
    webServer.send(404, "application/json", "");
    webServer.sendContent("{\"error\":\"UNKNOWN_DEVICE\"}");
    webServer.sendContent("");
    return;
  }

  String cmnd = sTopic.substring(base.length() - 1);
  if (cmnd.equalsIgnoreCase("/off")) {
    webServer.send(200, "application/json", "");
    webServer.sendContent("{\"result\":");
    webServer.sendContent(m171sOff() ? "1" : "0");
    webServer.sendContent("}");

  } else if (cmnd.equalsIgnoreCase("/boil")) {
    uint8_t temp = webServer.arg("temp").toInt();
    bool res = temp > 0 ? m171sBoilAndHeat(temp) : m171sBoil();
    webServer.send(200, "application/json", "");
    webServer.sendContent("{\"result\":");
    webServer.sendContent(res ? "1" : "0");
    webServer.sendContent("}");

  } else if (cmnd.equalsIgnoreCase("/heat")) {
    uint8_t temp = webServer.arg("temp").toInt();
    webServer.send(200, "application/json", "");
    webServer.sendContent("{\"result\":");
    webServer.sendContent(m171sHeat(temp) ? "1" : "0");
    webServer.sendContent("}");

  } else if (cmnd.equalsIgnoreCase("/state")) {
    webServer.send(200, "application/json", "");
    webServer.sendContent("{\"result\":");
    webServer.sendContent(getStatusJson(pServerAddress));
    webServer.sendContent("}");

  } else {
    webServer.send(404, "application/json", "");
    webServer.sendContent("{\"error\":\"UNKNOWN_COMMAND\"}");
  }
  webServer.sendContent("");
}

// MQTT funcitons
String mqttBaseTopic(BLEAddress* addr) {
  String base = String(MQTT_BASE_TOPIC);
  if (addr) {
    String sAddr = String(addr->toString().c_str());
    sAddr.replace(":", "");
#ifdef MQTT_UPPERCASE_DEV_TOPIC
    sAddr.toUpperCase();
#endif
    base = String(base + "/") + sAddr;
  }
  return base;
}

bool mqttPublish(BLEAddress* addr, const char* topic, const char* payload) {
  String base = mqttBaseTopic(addr);
  return mqttClient.publish(String(base + topic).c_str(), payload);
}

bool mqttSubscription(BLEAddress* addr, bool on) {
  String base = mqttBaseTopic(addr) + String(MQTT_CMND_TOPIC "/#");
  if (on) {
    if (mqttClient.subscribe(base.c_str())) {
      log_d("MQTT Subscribed to %s", base.c_str());
      return true;
    }
  } else {
    if (mqttClient.unsubscribe(base.c_str())) {
      log_d("MQTT Unsubscribed from %s", base.c_str());
      return true;
    }
  }
  return false;
}

void mqttConnected() {
  mqttClient.subscribe(MQTT_BASE_TOPIC "/monitoring");
  if (pServerAddress) {
    mqttSubscription(pServerAddress, true);
  }
}

void mqttCommand(const char* topic, const char* payload) {
  String sTopic = String(topic);

  if (sTopic.equalsIgnoreCase(MQTT_BASE_TOPIC "/monitoring")) {
    String ival = String(payload);
    if (ival.length()) {
      r4sStatusUpdateTime = ival.toInt() * 1000;
    }
    mqttPublish(0, MQTT_STAT_TOPIC "/monitoring", String((long)(r4sStatusUpdateTime / 1000)).c_str());
    return;
  }

  if (!pServerAddress || !pBLEClient->isConnected()) {
    sTopic.replace(MQTT_CMND_TOPIC "/", MQTT_ERROR_TOPIC "/");
    mqttClient.publish(sTopic.c_str(), "NOT_CONNECTED");
    return;
  }

  String base = mqttBaseTopic(pServerAddress) + String(MQTT_CMND_TOPIC "/");
  if (!sTopic.startsWith(base)) {
    sTopic.replace(MQTT_CMND_TOPIC "/", MQTT_ERROR_TOPIC "/");
    mqttClient.publish(sTopic.c_str(), "UNKNOWN_DEVICE");
    return;
  }

  String cmnd = sTopic.substring(base.length() - 1);
  String stat = String(MQTT_STAT_TOPIC) + cmnd;
  if (cmnd.equalsIgnoreCase("/off")) {
    mqttPublish(pServerAddress, stat.c_str(), m171sOff() ? "1" : "0");

  } else if (cmnd.equalsIgnoreCase("/boil")) {
    uint8_t temp = String(payload).toInt();
    bool res = temp > 0 ? m171sBoilAndHeat(temp) : m171sBoil();
    mqttPublish(pServerAddress, stat.c_str(), res ? "1" : "0");
    publishStatus(pServerAddress);

  } else if (cmnd.equalsIgnoreCase("/heat")) {
    uint8_t temp = String(payload).toInt();
    mqttPublish(pServerAddress, stat.c_str(), m171sHeat(temp) ? "1" : "0");
    publishStatus(pServerAddress);

  } else if (cmnd.equalsIgnoreCase("/state")) {
    publishStatus(pServerAddress);

  } else {
    sTopic.replace(MQTT_CMND_TOPIC "/", MQTT_ERROR_TOPIC "/");
    mqttClient.publish(sTopic.c_str(), "UNKNOWN_COMMAND");
    return;
  }

}

bool publishStatus(BLEAddress* addr) {
  String json = getStatusJson(addr);
  if (json.length()) {
    return mqttPublish(addr, MQTT_STAT_TOPIC "/state", json.c_str());
  }
  return false;
}


// R4S KETTLE Authorize
bool authorize(uint8_t retires) {
  r4scounter = 0;
  while (retires-- && r4sAuthorize()) {
    if (authorized)
      return true;
    delay(500);
  }
  return false;
}


void setupWiFi() {
  // We start by connecting to a WiFi network
  log_d("\nConnecting to %s", ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  log_i("\nWiFi connected\nIP address: %s", WiFi.localIP().toString().c_str());

  if (MDNS.begin("esp32")) {
    log_d("MDNS responder started");
  }
}
