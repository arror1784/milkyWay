#ifndef ESP32_LIB_SEN0153_H
#define ESP32_LIB_SEN0153_H

#include <HardwareSerial.h>

#define SEN0153_CMD_READ_DISTANCE       ((uint8_t)0x02)
#define SEN0153_CMD_READ_TEMPERATURE    ((uint8_t)0x03)
#define SEN0153_CMD_SET_ADDRESS         ((uint8_t)0x55)
#define SEN0153_CMD_SET_BAUD_RATE       ((uint8_t)0x08)
#define SEN0153_CMD_HEADER1             ((uint8_t)0x55)
#define SEN0153_CMD_HEADER2             ((uint8_t)0xAA)

#define SEN0153_ADDRESS_UNIVERSIAL      ((uint8_t)0xAB)
#define SEN0153_ADDRESS_DEFAULT         ((uint8_t)0x11)

#define SEN0153_RESULT_SUCCESS          ((uint8_t)0xCC)
#define SEN0153_RESULT_FAIL             ((uint8_t)0xEE)

#define SEN0153_BAUD_RATE_1200          ((unsigned long)1200)
#define SEN0153_BAUD_RATE_2400          ((unsigned long)2400)
#define SEN0153_BAUD_RATE_4800          ((unsigned long)4800)
#define SEN0153_BAUD_RATE_9600          ((unsigned long)9600)
#define SEN0153_BAUD_RATE_14400         ((unsigned long)14400)
#define SEN0153_BAUD_RATE_19200         ((unsigned long)19200)
#define SEN0153_BAUD_RATE_28800         ((unsigned long)28800)
#define SEN0153_BAUD_RATE_38400         ((unsigned long)38400)
#define SEN0153_BAUD_RATE_57600         ((unsigned long)57600)
#define SEN0153_BAUD_RATE_115200        ((unsigned long)115200)
#define SEN0153_BAUD_RATE_128000        ((unsigned long)128000)
#define SEN0153_BAUD_RATE_256000        ((unsigned long)256000)
#define SEN0153_BAUD_RATE_DEFAULT       SEN0153_BAUD_RATE_19200

#define SEN0153_BAUD_VAL_1200           ((uint8_t)0x00)
#define SEN0153_BAUD_VAL_2400           ((uint8_t)0x01)
#define SEN0153_BAUD_VAL_4800           ((uint8_t)0x02)
#define SEN0153_BAUD_VAL_9600           ((uint8_t)0x03)
#define SEN0153_BAUD_VAL_14400          ((uint8_t)0x04)
#define SEN0153_BAUD_VAL_19200          ((uint8_t)0x05)
#define SEN0153_BAUD_VAL_28800          ((uint8_t)0x06)
#define SEN0153_BAUD_VAL_38400          ((uint8_t)0x07)
#define SEN0153_BAUD_VAL_57600          ((uint8_t)0x08)
#define SEN0153_BAUD_VAL_115200         ((uint8_t)0x09)
#define SEN0153_BAUD_VAL_128000         ((uint8_t)0x0A)
#define SEN0153_BAUD_VAL_256000         ((uint8_t)0x0B)
#define SEN0153_BAUD_VAL_DEFAULT        SEN0153_BAUD_VAL_19200

class SEN0153 {
public:
    SEN0153(int8_t rx, int8_t tx, unsigned long baudRate = SEN0153_BAUD_RATE_DEFAULT);
    void begin();
    uint16_t readDistance(uint8_t address = SEN0153_ADDRESS_DEFAULT);
    float_t readTemperature(uint8_t address = SEN0153_ADDRESS_DEFAULT);
    bool setAddress(uint8_t oldAddress = SEN0153_ADDRESS_UNIVERSIAL, uint8_t newAddress = SEN0153_ADDRESS_DEFAULT);
    bool setBaudRate(uint8_t address = SEN0153_ADDRESS_DEFAULT, unsigned long baudRate = SEN0153_BAUD_RATE_DEFAULT);
private:
    void send(uint8_t address, uint8_t cmd, uint8_t dLen, uint8_t data[]);
    bool recv(uint8_t &address, uint8_t &cmd, uint8_t &dLen, uint8_t data[]);
    HardwareSerial serial;
    unsigned long baudRate;
    int8_t rx;
    int8_t tx;
};

#endif //ESP32_LIB_SEN0153_H