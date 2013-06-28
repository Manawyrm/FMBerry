#include "i2c.h"
/*
FMBerry - an cheap and easy way of transmitting music with your Pi.
Written 2013 by Tobias Mädel (t.maedel@alfeld.de)

Thanks to Rsoft for helping me preserving my sanity in spite of my non-existant knowledge of C (writing of Makefile and C-Headers)
Thanks to Paul Griffiths (1999) for his TCP socket helper.
*/

static int fd = -1;
static const char *fileName = "/dev/i2c-1";						
static int  address = 0x66;			

int TWI_init()
{
	if (fd >= 0)
		return 0; /* file already openned */
		
	if ((fd = open(fileName, O_RDWR)) < 0)
	{
		printf("Failed to open i2c port\n");
		return -1;
	}
	
	if (ioctl(fd, I2C_SLAVE, address) < 0) 
	{
		printf("Unable to get bus access to talk to slave\n");
		return -1;
	}
	
	return 0;
}

int TWI_writeData(uint8_t *data, uint32_t len)
{
	if (write(fd, data, len) != len) {
		return -1;
	}
	return 0;
}

int TWI_send(uint8_t addr, uint8_t data)
{		
	char buf[2];										
	buf[0] = addr;					
	buf[1] = data;
	if ((write(fd, buf, 2)) != 2) {
		return -1;
	}
	return 0;
}

int TWI_send_word(uint8_t addr, uint8_t data0, uint8_t data1)
{
	char buf[3];										
	buf[0] = addr;					
	buf[1] = data0;
	buf[2] = data1;
	if ((write(fd, buf, 3)) != 3) {
		return -1;
	}
	return 0;
}