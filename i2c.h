#ifndef __I2C_H_FMBERRY_
#define __I2C_H_FMBERRY_

#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int TWI_init();
int TWI_send_word(uint8_t addr, uint8_t data0, uint8_t data1);
int TWI_send(uint8_t addr, uint8_t data);
int TWI_writeData(uint8_t *data, uint32_t len);

#endif
