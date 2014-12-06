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
#include "i2c.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include <memory.h>
#include <linux/i2c-dev.h>

int i2c_init(uint8_t bus, uint8_t address)
{
	int fd;
	char bus_name[64];
		
	sprintf(bus_name, "/dev/i2c-%u", bus);

	if ((fd = open(bus_name, O_RDWR)) < 0)
	{
		printf("Failed to open i2c port\n");
		return -1;
	}
	
	if (i2c_select(fd, address) < 0) {
		printf("Unable to get bus access to talk to slave\n");
		close(fd);
		return -1;
	}
	
	return fd;
}

int i2c_select(int dev, uint8_t address)
{
	return ioctl(dev, I2C_SLAVE, address);
}

int i2c_send_data(int dev, uint8_t addr, uint8_t *data, uint32_t len)
{
	uint8_t *buf = (uint8_t *)alloca(len+1);
	memcpy(buf +1, data, len);
	buf[0] = addr;
	len++;

	if (write(dev, buf, len) != len)
		return -1;
	return 0;
}

int i2c_send(int dev, uint8_t addr, uint8_t data)
{		
	char buf[2];										
	buf[0] = addr;					
	buf[1] = data;
	if ((write(dev, buf, 2)) != 2) {
		return -1;
	}
	return 0;
}

// send 16 bit, MSB first
int i2c_send_word(int dev, uint8_t addr, uint8_t *data)
{
	char buf[3];										
	buf[0] = addr;					
	buf[1] = data[1];
	buf[2] = data[0];
	if ((write(dev, buf, 3)) != 3) {
		return -1;
	}
	return 0;
}
