bool m171sOff() {
  if (r4sCommand(0x04) != 5)
    return false;

  return notifyData[3] == 1;
}

bool m171sOn(bool boil, uint8_t temp) {
  uint8_t data[] = { boil ? (uint8_t)0 : (uint8_t)1, 0, temp, 0 };
  if (r4sCommand(0x05, &data[0], sizeof(data)) != 5)
    return false;

  return notifyData[3] == 1;
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

