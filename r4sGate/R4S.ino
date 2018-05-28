#define R4S_READ_TIMEOUT_SECS 3

static boolean authorized = false;
static uint8_t r4scounter = 0;

uint8_t r4sWrite(uint8_t cmd, uint8_t* data = 0, size_t len = 0) {

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
    Serial.print(">> ");
    for (int i = 0 ; i < sz; i++) {
      Serial.print(*(buffer + i), HEX);
      Serial.print(" ");
    }
    Serial.println();
#endif
  } catch (...) {
    Serial.println("writeValue failed");
  }
  return r4scounter++;
}

int8_t r4sCommand(uint8_t cmd, uint8_t* data = 0, size_t len = 0) {
  uint16_t timeout = R4S_READ_TIMEOUT_SECS * 100; // 3 seconds
  notifyDataLen = -1;

  uint8_t cnt = r4sWrite(cmd, data, len);
  while (--timeout && (notifyDataLen == -1)) {
    delay(10);
  }

#ifdef R4S_LOG_EXCHANGE
  Serial.print("<< ");
  for (int i = 0 ; i < notifyDataLen; i++) {
    Serial.print(*(notifyData + i), HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif

  if (notifyDataLen > 1 && notifyData[1] != cnt)
    return -3; // counter mismatch

  return notifyDataLen;
}

bool r4sAuthorize() {
  if (r4sCommand(0xff, &r4sAuth[0], sizeof(r4sAuth)) != 5)
    return false;

  authorized = notifyData[3] == 1;
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

