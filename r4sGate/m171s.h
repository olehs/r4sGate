#ifndef M171S_H
#define M171S_H

class M171SStatus {
  public:
    M171SStatus(uint8_t* data = 0) {
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

    M171SStatus(const M171SStatus& other) {
      copyFrom(other);
    }

    M171SStatus& operator =(const M171SStatus& other) {
      copyFrom(other);
      return *this;
    }

    bool isValid() {
      return m_valid;
    }

    uint8_t program;
    uint8_t state;
    uint8_t error;
    bool isHeating;
    uint8_t remainingHours;
    uint8_t remainingMinutes;
    uint8_t targetTemperature;
    uint8_t currentTemperature;

  private:
    void copyFrom(const M171SStatus& other) {
      m_valid = other.m_valid;
      program = other.program;
      targetTemperature = other.targetTemperature;
      remainingHours = other.remainingHours;
      remainingMinutes = other.remainingMinutes;
      isHeating = other.isHeating;
      error = other.error;
      currentTemperature = other.currentTemperature;
    }

    boolean m_valid;
};

#endif
