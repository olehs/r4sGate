#include "R4S.h"

#include "r4scfg.h"

uint8_t r4scounter = 0;
bool authorized = false;

uint8_t r4sWrite(uint8_t cmd, uint8_t* data, size_t len) {

  size_t sz = 4 + len; // 55, counter, cmd, AA
  uint8_t* buffer = new uint8_t[sz];
  buffer[0] = 0x55;
  buffer[1] = r4scounter;
  buffer[2] = cmd;
  buffer[sz - 1] = 0xAA;
  if (len > 0) {
    memcpy(&buffer[3], data, len);
  }

  try {
    pRemoteTXCharacteristic->writeValue(buffer, sz);

#ifdef R4S_LOG_EXCHANGE
    log_d(">> %s", BLEUtils::buildHexData(NULL, buffer, sz));
#endif
  } catch (...) {
    log_e("writeValue failed");
  }
  return r4scounter++;
}

int8_t r4sCommand(uint8_t cmd, uint8_t* data, size_t len) {
  uint16_t timeout = R4S_READ_TIMEOUT_SECS * 100; // 3 seconds
  notifyDataLen = -1;

  uint8_t cnt = r4sWrite(cmd, data, len);
  while (--timeout && (notifyDataLen == -1)) {
    delay(10);
  }

#ifdef R4S_LOG_EXCHANGE
  log_d("<< %s", BLEUtils::buildHexData(NULL, notifyData, notifyDataLen));
#endif

  if (notifyDataLen > 1 && notifyData[1] != cnt)
    return -3; // counter mismatch

  return notifyDataLen;
}

bool r4sAuthorize() {
  if (r4sCommand(0xff, r4sAuth, sizeof(r4sAuth)) != 0x05)
    return false;

  authorized = (notifyData[3] == 1);
  return true;
}

uint16_t r4sVersion() {
  if (r4sCommand(0x01) != 6)
    return 0;

  uint16_t ver = notifyData[3];
  ver <<= 8;
  ver += notifyData[4];
  return ver;
}
