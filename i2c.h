#ifndef __I2C_H_FMBERRY_
#define __I2C_H_FMBERRY_

#include <stdint.h>

int i2c_init(uint8_t bus, uint8_t address);
int i2c_select(int dev, uint8_t address);
int i2c_send_word(int dev, uint8_t addr, uint8_t data0, uint8_t data1);
int i2c_send(int dev, uint8_t addr, uint8_t data);
int i2c_writeData(int dev, uint8_t *data, uint32_t len);

#endif
