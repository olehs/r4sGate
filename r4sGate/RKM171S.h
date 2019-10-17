#ifndef RKM171S_H
#define RKM171S_H

#include "R4S.h"

class M171SStatus {
  public:
    M171SStatus(uint8_t* data = 0);
    M171SStatus(const M171SStatus& other);
    M171SStatus& operator =(const M171SStatus& other);

    bool isValid();

    uint8_t program;
    uint8_t state;
    uint8_t error;
    bool isHeating;
    uint8_t remainingHours;
    uint8_t remainingMinutes;
    uint8_t targetTemperature;
    uint8_t currentTemperature;

  private:
    void copyFrom(const M171SStatus& other);

    bool m_valid;
};

bool m171sOff();
bool m171sOn(bool boil, uint8_t temp);
bool m171sBoil();
bool m171sHeat(uint8_t temp);
bool m171sBoilAndHeat(uint8_t temp);
M171SStatus m171sStatus();

String getStatusJson(BLEAddress* addr);

#endif
