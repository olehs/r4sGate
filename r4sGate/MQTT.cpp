#include "MQTT.h"

#include "BLEUtils.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  std::string str = BLEUtils::buildPrintData(payload, length);
  log_i("Message arrived [%s] %s", topic, str.c_str());

  mqttCommand(topic, str.c_str());
}

void reconnectMQTT() {
  if (String(mqtt_server).length() == 0)
    return;

  while (!mqttClient.connected()) {
    log_d("Attempting MQTT connection...");

    if (mqttClient.connect(mqtt_client)) {
      log_d("connected");

      mqttClient.publish(MQTT_BASE_TOPIC "/status", "online");
      mqttConnected();
    } else {
      log_e("failed, rc=%d. Try again in 5 seconds", mqttClient.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setupMQTT() {
  if (String(mqtt_server).length() == 0)
    return;
    
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}

void loopMQTT() {

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
}
