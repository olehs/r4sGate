#ifndef R4S_H
#define R4S_H

#include "BLE.h"

#define R4S_READ_TIMEOUT_SECS 3

extern uint8_t r4scounter;
extern bool authorized;

uint8_t r4sWrite(uint8_t cmd, uint8_t* data = 0, size_t len = 0);
int8_t r4sCommand(uint8_t cmd, uint8_t* data = 0, size_t len = 0);
bool r4sAuthorize();
uint16_t r4sVersion();

#endif
