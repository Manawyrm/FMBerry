/*
	FMBerry - an cheap and easy way of transmitting music with your Pi.
    Copyright (C) 2011-2013 by Tobias Mädel (t.maedel@alfeld.de)
	Copyright (C) 2013-2014 by Andrey Chilikin (https://github.com/achilikin)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __I2C_H_FMBERRY_
#define __I2C_H_FMBERRY_

#include <stdint.h>

int i2c_init(uint8_t bus, uint8_t address);
int i2c_select(int dev, uint8_t address);
int i2c_send_word(int dev, uint8_t addr, uint8_t *data);
int i2c_send(int dev, uint8_t addr, uint8_t data);
int i2c_send_data(int dev, uint8_t addr, uint8_t *data, uint32_t len);

#endif
