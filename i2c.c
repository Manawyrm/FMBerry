#include "i2c.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>

/*
FMBerry - an cheap and easy way of transmitting music with your Pi.
Written 2013 by Tobias Mädel (t.maedel@alfeld.de)

Thanks to Rsoft for helping me preserving my sanity in spite of my non-existant knowledge of C (writing of Makefile and C-Headers)
Thanks to Paul Griffiths (1999) for his TCP socket helper.
*/

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

int i2c_writeData(int dev, uint8_t *data, uint32_t len)
{
	if (write(dev, data, len) != len) {
		return -1;
	}
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

int i2c_send_word(int dev, uint8_t addr, uint8_t data0, uint8_t data1)
{
	char buf[3];										
	buf[0] = addr;					
	buf[1] = data0;
	buf[2] = data1;
	if ((write(dev, buf, 3)) != 3) {
		return -1;
	}
	return 0;
}