
#define MQTT_BASE_TOPIC "r4s"
#define MQTT_CMND_TOPIC "/cmnd"
#define MQTT_STAT_TOPIC "/stat"
#define MQTT_ERROR_TOPIC "/error"

#ifndef R4SGATE_NO_MQTT

void mqttConnected();
void mqttCommand(const char* topic, const char* payload);

WiFiClient espClient;
PubSubClient mqttClient(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  char data[length + 1];
  memcpy(&data[0], payload, length);
  data[length] = '\0';
  mqttCommand(topic, &data[0]);
}

void reconnectMQTT() {

  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (mqttClient.connect(mqtt_client)) {
      Serial.println("connected");

      mqttClient.publish(MQTT_BASE_TOPIC "/status", "online");
      mqttConnected();
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setupMQTT() {
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}

void loopMQTT() {

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
}

#endif //R4SGATE_NO_MQTT
