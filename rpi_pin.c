/*
	The MIT License (MIT)

	Copyright (c) 2013 Andrey Chilikin (achilikin@gmail.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "rpi_pin.h"

#if _DEBUG
#define _IFDEB(x) do { x; } while(0)
#else
#define _IFDEB(x)
#endif

static const char *irq_mode[] = { "none\n", "rising\n", "falling\n", "both\n" };

static uint8_t valid_pins_r1[64] =
{
//  pin index
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	1,1,0,0,1,0,0,1,1,1,1,1,0,0,1,1,0,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0
};

static uint8_t valid_pins_r2[64] =
{
//  pin index
//  0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	0,0,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,1,1,0,0,0,1,1,1,1,0,1,1,1,1,1
};

static uint8_t *pvalid_pins = valid_pins_r2;

static int pin_fds[64];

#define FPIN_EXPORTED  0x0001
#define FPIN_DIR_INPUT 0x0020

static int pin_flags[64];

int rpi_pin_init(int pi_revision)
{
	if (pi_revision == 1)
		pvalid_pins = valid_pins_r1;

	return 0;
}

int rpi_pin_export(uint8_t pin, enum PIN_DIRECTION dir)
{
	FILE *fd;
	char file[128];

	if ((pin > 63) || !pvalid_pins[pin]) {
		_IFDEB(fprintf(stderr, "%s: invalid pin #%u\n", __func__, pin));
		return -1;
	}

	if ((fd = fopen ("/sys/class/gpio/export", "w")) == NULL) {
		_IFDEB(fprintf(stderr, "%s: unable to export pin #%u\n", __func__, pin));
		return -1;
	}
	fprintf(fd, "%d\n", pin);
	fclose(fd);

	sprintf(file, "/sys/class/gpio/gpio%d/direction", pin);
	if ((fd = fopen (file, "w")) == NULL) {
		_IFDEB(fprintf(stderr, "%s: unable to set direction for pin #%u\n", __func__, pin));
		return -1;
	}

	pin_flags[pin] |= FPIN_EXPORTED;
	if (dir == RPI_INPUT)
		pin_flags[pin] |= FPIN_DIR_INPUT;
	fprintf(fd, (dir == RPI_INPUT) ? "in\n" : "out\n");
	fclose(fd);

	sprintf(file, "/sys/class/gpio/gpio%d/value", pin);
	pin_fds[pin] = open(file, O_RDWR);

	return 0;
}

int rpi_pin_set_dir(uint8_t pin, enum PIN_DIRECTION dir)
{
	FILE *fd;
	char file[128];

	if ((pin > 63) || !pvalid_pins[pin])
		return -1;
	
	if (!(pin_flags[pin] & FPIN_EXPORTED))
		return -1;

	sprintf(file, "/sys/class/gpio/gpio%d/direction", pin);
	if ((fd = fopen (file, "w")) == NULL) {
		_IFDEB(fprintf(stderr, "%s: unable to set direction for pin #%u\n", __func__, pin));
		return -1;
	}

	if (dir == RPI_INPUT) {
		pin_flags[pin] |= FPIN_DIR_INPUT;
		fprintf(fd, "in\n");
	}
	else {
		pin_flags[pin] &= ~FPIN_DIR_INPUT;
		fprintf(fd, "out\n");
	}
	fclose(fd);

	return 0;
}

// read input value:
//	0 - low
//	1 - high
// -1 - error
int rpi_pin_get(uint8_t pin)
{
	// this will check pin validity as well
	int fd = rpi_pin_fd(pin);

	if (fd < 0)
		return -1;

	if (!(pin_flags[pin] & FPIN_DIR_INPUT))
		return -1;

	return rpi_pin_poll_clear(fd);
}

// set output value
int rpi_pin_set(uint8_t pin, uint8_t value)
{
	// this will check pin validity as well
	int fd = rpi_pin_fd(pin);

	if (fd < 0)
		return -1;

	if (pin_flags[pin] & FPIN_DIR_INPUT)
		return -1;
	
	lseek(fd, 0, SEEK_SET);
	return (write(fd, value ? "1" : "0", 1) == 1) ? 0 : -1;
}

int rpi_pin_unexport(uint8_t pin)
{
	FILE *fd;
	char file[128];

	if ((pin > 63) || !pvalid_pins[pin])
		return -1;

	if (!(pin_flags[pin] & FPIN_EXPORTED))
		return 0;

	if ((fd = fopen ("/sys/class/gpio/unexport", "w")) == NULL) {
		_IFDEB(fprintf(stderr, "%s: unable to unexport pin #%u\n", __func__, pin));
		return -1;
	}
	fprintf(fd, "%d\n", pin);
	fclose(fd);

	pin_flags[pin] = 0;
	close(pin_fds[pin]);
	pin_fds[pin] = -1;

	return 0;
}

// functions for pin change of value edge detection support using poll()

// returns pin's file descriptor
int rpi_pin_fd(uint8_t pin)
{
	if ((pin > 63) || !pvalid_pins[pin])
		return -1;

	if (pin_flags[pin] & FPIN_EXPORTED)
		return pin_fds[pin];

	return -1;
}

// enables POLLPRI on edge detection, pin must be in INPUT mode
// returns file descriptor to be used with poll() or -1 as error
int rpi_pin_poll_enable(uint8_t pin, enum PIN_EDGE_MODE mode)
{
	FILE *fd;
	char file[128];

	if ((pin > 63) || !pvalid_pins[pin])
		return -1;

	if (!(pin_flags[pin] & FPIN_EXPORTED))
		rpi_pin_export(pin, RPI_INPUT);

	if (pin_fds[pin] < 0)
		return -1;

	sprintf(file, "/sys/class/gpio/gpio%d/edge", pin);
	if ((fd = fopen (file, "w")) == NULL) {
		_IFDEB(fprintf(stderr, "%s: setting edge detection failed for pin #%u: %s\n", __func__, pin));
		return -1;
	}

	fputs(irq_mode[mode], fd);
	fclose(fd);

	rpi_pin_poll_clear(pin_fds[pin]);

	return pin_fds[pin];
}

