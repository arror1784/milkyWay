#include "SEN0153.h"

SEN0153::SEN0153(int8_t rx, int8_t tx, unsigned long baudRate) : rx(rx), tx(tx), baudRate(baudRate), serial(2) {}

void SEN0153::begin() {
    switch (baudRate) {
        case SEN0153_BAUD_RATE_1200:
        case SEN0153_BAUD_RATE_2400:
        case SEN0153_BAUD_RATE_4800:
        case SEN0153_BAUD_RATE_9600:
        case SEN0153_BAUD_RATE_14400:
        case SEN0153_BAUD_RATE_19200:
        case SEN0153_BAUD_RATE_28800:
        case SEN0153_BAUD_RATE_38400:
        case SEN0153_BAUD_RATE_57600:
        case SEN0153_BAUD_RATE_115200:
        case SEN0153_BAUD_RATE_128000:
        case SEN0153_BAUD_RATE_256000:
            break;
        default:
            baudRate = SEN0153_BAUD_RATE_DEFAULT;
    }

    serial.begin(baudRate, SERIAL_8N1, rx, tx);
}

uint16_t SEN0153::readDistance(uint8_t address) {
    uint8_t address_recv, cmd_recv, dLen_recv, data_recv[2];

    send(address, SEN0153_CMD_READ_DISTANCE, 0x00, NULL);

    if (!recv(address_recv, cmd_recv, dLen_recv, data_recv))
        return 0;

    if (address != address_recv || cmd_recv != SEN0153_CMD_READ_DISTANCE || dLen_recv != 0x02)
        return 0;

    return (uint16_t) data_recv[0] << 8 | data_recv[1];
}

float_t SEN0153::readTemperature(uint8_t address) {
    uint8_t address_recv, cmd_recv, dLen_recv, data_recv[2];

    send(address, SEN0153_CMD_READ_TEMPERATURE, 0x00, NULL);

    if (!recv(address_recv, cmd_recv, dLen_recv, data_recv))
        return 0;

    if (address != address_recv || cmd_recv != SEN0153_CMD_READ_TEMPERATURE || dLen_recv != 0x02)
        return 0;

    return ((uint16_t) data_recv[0] << 8 | data_recv[1]) / (float_t) 10;
}

bool SEN0153::setAddress(uint8_t oldAddress, uint8_t newAddress) {
    uint8_t dLen_send = 0x01, data_send[1] = {newAddress};
    uint8_t address_recv, cmd_recv, dLen_recv, data_recv[2];

    send(oldAddress, SEN0153_CMD_SET_ADDRESS, dLen_send, data_send);

    if (!recv(address_recv, cmd_recv, dLen_recv, data_recv))
        return false;

    if (newAddress != address_recv || cmd_recv != SEN0153_CMD_SET_ADDRESS || dLen_recv != 0x01)
        return false;

    return data_recv[0] == SEN0153_RESULT_SUCCESS;
}

bool SEN0153::setBaudRate(uint8_t address, unsigned long baudRate) {
    uint8_t dLen_send = 0x01, data_send[1];
    uint8_t address_recv, cmd_recv, dLen_recv, data_recv[2];

    switch (baudRate) {
        case SEN0153_BAUD_RATE_1200:
            data_send[0] = SEN0153_BAUD_VAL_1200;
            break;
        case SEN0153_BAUD_RATE_2400:
            data_send[0] = SEN0153_BAUD_VAL_2400;
            break;
        case SEN0153_BAUD_RATE_4800:
            data_send[0] = SEN0153_BAUD_VAL_4800;
            break;
        case SEN0153_BAUD_RATE_9600:
            data_send[0] = SEN0153_BAUD_VAL_9600;
            break;
        case SEN0153_BAUD_RATE_14400:
            data_send[0] = SEN0153_BAUD_VAL_14400;
            break;
        case SEN0153_BAUD_RATE_19200:
            data_send[0] = SEN0153_BAUD_VAL_19200;
            break;
        case SEN0153_BAUD_RATE_28800:
            data_send[0] = SEN0153_BAUD_VAL_28800;
            break;
        case SEN0153_BAUD_RATE_38400:
            data_send[0] = SEN0153_BAUD_VAL_38400;
            break;
        case SEN0153_BAUD_RATE_57600:
            data_send[0] = SEN0153_BAUD_VAL_57600;
            break;
        case SEN0153_BAUD_RATE_115200:
            data_send[0] = SEN0153_BAUD_VAL_115200;
            break;
        case SEN0153_BAUD_RATE_128000:
            data_send[0] = SEN0153_BAUD_VAL_128000;
            break;
        case SEN0153_BAUD_RATE_256000:
            data_send[0] = SEN0153_BAUD_VAL_256000;
            break;
        default:
            baudRate = SEN0153_BAUD_RATE_DEFAULT;
            data_send[0] = SEN0153_BAUD_VAL_DEFAULT;
            break;
    }

    send(address, SEN0153_CMD_SET_BAUD_RATE, dLen_send, data_send);

    if (!recv(address_recv, cmd_recv, dLen_recv, data_recv))
        return false;

    if (address != address_recv || cmd_recv != SEN0153_CMD_SET_BAUD_RATE || dLen_recv != 0x01)
        return false;

    if (data_recv[0] == SEN0153_RESULT_SUCCESS) {
        serial.flush();
        serial.begin((uint32_t) baudRate, SERIAL_8N1, rx, tx);
        return true;
    }
    else {
        return false;
    }
}

void SEN0153::send(uint8_t address, uint8_t cmd, uint8_t dLen, uint8_t data[]) {
    uint8_t cmd_frame[8];

    switch (cmd) {
        case SEN0153_CMD_READ_DISTANCE:
        case SEN0153_CMD_READ_TEMPERATURE:
        case SEN0153_CMD_SET_ADDRESS:
        case SEN0153_CMD_SET_BAUD_RATE:
            break;
        default:
            return;
    }

    cmd_frame[0] = SEN0153_CMD_HEADER1;
    cmd_frame[1] = SEN0153_CMD_HEADER2;
    cmd_frame[2] = address;
    cmd_frame[3] = dLen;
    cmd_frame[4] = cmd;
    for (int i = 0; i < dLen; i++)
        cmd_frame[5 + i] = data[i];

    cmd_frame[5 + dLen] = 0;
    for (int i = 0; i < 5 + dLen; i++) {
        cmd_frame[5 + dLen] += cmd_frame[i];
        serial.write(cmd_frame[i]);
    }
    serial.write(cmd_frame[5 + dLen]);
}

bool SEN0153::recv(uint8_t &address, uint8_t &cmd, uint8_t &dLen, uint8_t data[]) {
    uint8_t cmd_frame[8], crc = 0;
    int wait_cnt = 0;

    while (serial.available() == 0 && wait_cnt < 10) {
        wait_cnt++;
        delay(10);
    }

    if (wait_cnt == 10)
        return false;

    cmd_frame[3] = 0;
    for (int i = 0; i < 5 + cmd_frame[3]; i++) {
        cmd_frame[i] = serial.read();
        crc += cmd_frame[i];
    }
    cmd_frame[5 + dLen] = serial.read();
    address = cmd_frame[2];
    dLen = cmd_frame[3];
    cmd = cmd_frame[4];
    for (int i = 0; i < dLen; i++)
        data[i] = cmd_frame[5 + i];

    return true;
    return crc == cmd_frame[5 + dLen];
}