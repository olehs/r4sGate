#include "RKM171S.h"

M171SStatus::M171SStatus(uint8_t* data) {
  m_valid = (data != 0);
  if (!m_valid)
    return;

  program = *(data + 3);
  targetTemperature = *(data + 5);
  remainingHours = *(data + 8);
  remainingMinutes = *(data + 9);
  isHeating = *(data + 10) == 1;
  state = *(data + 11);
  error = *(data + 12);
  currentTemperature = *(data + 13);
}

M171SStatus::M171SStatus(const M171SStatus& other) {
  copyFrom(other);
}

M171SStatus& M171SStatus::operator =(const M171SStatus& other) {
  copyFrom(other);
  return *this;
}


bool M171SStatus::isValid() {
  return m_valid;
}

void M171SStatus::copyFrom(const M171SStatus& other) {
  m_valid = other.m_valid;
  program = other.program;
  targetTemperature = other.targetTemperature;
  remainingHours = other.remainingHours;
  remainingMinutes = other.remainingMinutes;
  isHeating = other.isHeating;
  error = other.error;
  currentTemperature = other.currentTemperature;
}

String getStatusJson(BLEAddress* addr) {
  String json;
  M171SStatus st = m171sStatus();
  if (st.isValid()) {
    json = String("{");
    json += String("\"temp\":") + String(st.currentTemperature) + String(",");
    json += String("\"target\": ") + String(st.targetTemperature) + String(",");
    json += String("\"heat\": ") + String(st.isHeating ? "1" : "0") + String(",");
    json += String("\"state\": ") + String(st.state) + String(",");
    json += String("\"hours\": ") + String(st.remainingHours) + String(",");
    json += String("\"mins\": ") + String(st.remainingMinutes) + String(",");
    json += String("\"prog\": ") + String(st.program) + String(",");
    json += String("\"error\": ") + String(st.error);
    json += String("}");
  }
  return json;
}

bool m171sOff() {
  if (r4sCommand(0x04) != 5)
    return false;

  return notifyData[3] == 1;
}

bool m171sOn(bool boil, uint8_t temp) {
  uint8_t data[] = { boil ? (uint8_t)0 : (uint8_t)1, 0, temp, 0 };
  if (r4sCommand(0x05, data, sizeof(data)) != 5)
    return false;

  if (notifyData[3] == 1)
    return false;

#ifdef R4S_G200S_SUPPORT
  if (r4sCommand(0x03) != 5)
    return false;
  if (notifyData[3] == 1);
#endif

  return true;
}

bool m171sBoil() {
  return m171sOn(true, 0);
}

bool m171sHeat(uint8_t temp) {
  return m171sOn(false, temp);
}

bool m171sBoilAndHeat(uint8_t temp) {
  return m171sOn(true, temp);
}

M171SStatus m171sStatus() {
  if (r4sCommand(0x06) != 20)
    return M171SStatus();

  return M171SStatus(notifyData);
}
