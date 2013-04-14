#include "i2c.h"
/*
FMBerry - an cheap and easy way of transmitting music with your Pi.
Written 2013 by Tobias Mädel (t.maedel@alfeld.de)

Thanks to Rsoft for helping me preserving my sanity in spite of my non-existant knowledge of C (writing of Makefile and C-Headers)
Thanks to Paul Griffiths (1999) for his TCP socket helper.
*/

int fd;													
char *fileName = "/dev/i2c-1";						
int  address = 0x66;			

int TWI_init()
{
	if ((fd = open(fileName, O_RDWR)) < 0)
	{
		printf("Failed to open i2c port\n");
		exit(1);
	}
	
	if (ioctl(fd, I2C_SLAVE, address) < 0) 
	{
		printf("Unable to get bus access to talk to slave\n");
		exit(1);
	}
}

uint8_t init_data[] =
{
	0x00, 0x02, 0x83, 0x0A, 
	0x00, 0x00, 0x00, 0x00, 
	0x7E, 0x0E, 0x08, 0x3F, 
	0x2A, 0x0C, 0xE6, 0x3F, 
	0x70, 0x0A, 0xE4, 0x00,
	0x42, 0xC0, 0x41, 0xF4
};
int TWI_writeInitData()
{
	if ((write(fd, init_data, sizeof(init_data))) != sizeof(init_data)) {
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