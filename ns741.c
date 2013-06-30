#include "ns741.h"
#include "i2c.h"

/*
FMBerry - an cheap and easy way of transmitting music with your Pi.
Written 2013 by Tobias Mädel (t.maedel@alfeld.de)

Thanks to Rsoft for helping me preserving my sanity in spite of my non-existant knowledge of C (writing of Makefile and C-Headers)
Thanks to Paul Griffiths (1999) for his TCP socket helper.
*/

// I2C address of MMR-70
static const int address = 0x66;
static int i2cbus = -1;

//                        "----------------------------64----------------------------------"
char radiotext_text[64] = "                                                                ";
// *hrr hrr*              "Twilight Sparkle is best Pony.                                  "

/* 4 RDS 0A Groups containing Program service name */
static uint8_t station_frames[16][3] =
{
	{0x03, 0xE0, 0x00},
	{0x05, 0x01, 0x24},
	{0x05, 0xE0, 0xCD},
	{0x05, ' ' , ' ' },	// 1  2

	{0x03, 0xE0, 0x00},
	{0x05, 0x01, 0x21},
	{0x05, 0xE0, 0xCD},
	{0x05, ' ' , ' ' }, // 3  4

	{0x03, 0xE0, 0x00},
	{0x05, 0x01, 0x22},
	{0x05, 0xE0, 0xCD},
	{0x05, ' ' , ' ' }, // 5  6

	{0x03, 0xE0, 0x00},
	{0x05, 0x01, 0x27},
	{0x05, 0xE0, 0xCD},
	{0x05, ' ' , ' ' }, // 7  8
};

// Radiotext
/* 1 RDS 2A Group containing Radiotext chars */
static uint8_t text_frames[4][3] =
{
	{0x03, 0xE0, 0x00},
	{0x05, 0x21, 0x20},
	{0x05, ' ' , ' ' },	// 1  2
	{0x05, ' ' , ' ' },	// 3  4
};

void ns741_rds_set_radiotext(const char *text)
{
	uint8_t i;

	// first copy text to our buffer
	for (i = 0; *text && (i < 64); i++, text++)
	{
		radiotext_text[i] = *text;
	}
	// pad buffer with spaces
	for (; i < 64; i++)
	{
		radiotext_text[i] = ' ';
	}
}

// text - up to 8 characters padded with zeros
// zeros will be replaced with spaces for transmission
// so exactly 8 chars will be transmitted
void ns741_rds_set_progname(const char *text)
{
	station_frames[3][1] = text[0] ? text[0] : ' ';
	station_frames[3][2] = text[1] ? text[1] : ' ';
	station_frames[7][1] = text[2] ? text[2] : ' ';
	station_frames[7][2] = text[3] ? text[3] : ' ';
	station_frames[11][1]= text[4] ? text[4] : ' ';
	station_frames[11][2]= text[5] ? text[5] : ' ';
	station_frames[15][1]= text[6] ? text[6] : ' ';
	station_frames[15][2]= text[7] ? text[7] : ' ';
}

// register 0x00 controls power and oscillator:
//	bit 0 - power
//	bit 1 - oscillator
void ns741_power(uint8_t on)
{
	uint8_t reg00h;
	if (on)
		reg00h = 0x03;	// power + oscillator are active
	else
		reg00h = 0x02;	// power is off

	i2c_send(i2cbus, 0x00, reg00h);
	return;
}

/************************************************************************ /
 * Write initial Data to NS741
 *
 * Thanks to Silvan König for init-sequence
 *************************************************************************/
static uint8_t init_data[] =
{
	0x00, // start address 0
	// default map for all 22 RW registers
	0x02, 0x83, 0x0A, 0x00,
	0x00, 0x00, 0x00, 0x7E,
	0x0E, 0x08, 0x3F, 0x2A,
	0x0C, 0xE6, 0x3F, 0x70,
	0x0A, 0xE4, 0x00, 0x42,
	0xC0, 0x41, 0xF4
};

int ns741_init(uint8_t bus)
{
	i2cbus = i2c_init(bus, address);
	if ((i2cbus == -1) || (i2c_send(i2cbus, 0x00, 0x00) == -1))
	{
		return -1;
	}

	// reset registers to default values
	i2c_writeData(i2cbus, init_data, sizeof(init_data));

	i2c_send(i2cbus, 0x02, 0x0B);
	i2c_send(i2cbus, 0x15, 0x11);
//	i2c_send(i2cbus, 0x10, 0xE0); // we will turn on RDS signal depending on config file

	// Configuration
	i2c_send(i2cbus, 0x07, 0x7E);
	i2c_send(i2cbus, 0x08, 0x0E);
	i2c_send(i2cbus, 0x02, 0xCA); // unmute and set txpwr to 2.0 mW
	i2c_send(i2cbus, 0x01, 0x81);
	
	return 0;
}

// register 0x01 controls stereo subcarrier and pilot: 
//  bit 0: pre-emphasis on = 1
//  bit 1: pre-emphasis selection: 0 - 50us, 1 - 75us
//	bit 4: set to 1 to turn off subcarrier
//	bit 7: set to 0 to turn off pilot tone
// default: 0x81
static uint8_t reg01h = 0x81;

void ns741_stereo(uint8_t on)
{
	if (on) {
		reg01h &= ~0x10; // enable subcarrier
		reg01h |= 0x80;  // enable pilot tone
	}
	else {
		reg01h |= 0x10;  // disable subcarrier
		reg01h &= ~0x80; // disable pilot tone
	}

	i2c_send(i2cbus, 0x01, reg01h);

	return;
}

// register 0x02 controls output power and mute: 
//	RF output power 0-3 (bits 7-6) corresponding to 0.5, 0.8, 1.0 and 2.0 mW
//	reserved bits 1 & 3 are set
//	mute is off (bit 0)
// default: 0xCA
static uint8_t reg02h = 0xCA;

void ns741_mute(uint8_t on)
{
	if (on) {
		reg02h |= 1;
	}
	else {
		reg02h &= ~1;
	}
	i2c_send(i2cbus, 0x02, reg02h);

	return;
}

void ns741_txpwr(uint8_t strength)
{
	// clear RF power bits: set power level 0 - 0.5mW
	reg02h &= ~0xC0;
	strength &= 0x03; // just in case normalize strength
	reg02h |= (strength << 6);

	i2c_send(i2cbus, 0x02, reg02h);

	return;
}

void ns741_set_frequency(uint32_t f_khz)
{
	/* calculate frequency in 8.192kHz steps*/
	uint16_t val = (uint16_t)((uint32_t)f_khz*1000ULL/8192ULL);

	// it is recommended to mute transmitter before changing frequency
	i2c_send(i2cbus, 0x02, reg02h | 0x01);

	i2c_send(i2cbus, 0x0A, val);
	i2c_send(i2cbus, 0x0B, val >> 8);

	// restore previous mute state
	i2c_send(i2cbus, 0x02, reg02h);

	//printf("\nSet frequency to  %lu(%lu) kHz \n", f_khz, val*8192UL/1000UL);
	return;
}

// register 0x10 controls RDS: 
//	reserved bits 0-5, with bit 5 set
//	bit 6: 0 - RDS off, 1 - RDS on
//  bit 7: RDS data format, 1 - with checkword
// default: 0x90
static uint8_t reg10h = 0x90;

void ns741_rds(uint8_t on)
{
	if (on) {
		reg10h |= 0x40;
	}
	else {
		reg10h &= ~0x40;
	}
	i2c_send(i2cbus, 0x10, reg10h);

	return;
}

static uint8_t frame_counter = 0;
static uint8_t radiotext_counter = 0;

// start RDS transmission
int ns741_rds_start(void)
{
	frame_counter = 0;
	radiotext_counter = 0;

	return i2c_send_word(i2cbus, station_frames[0][0],
		station_frames[0][1],
		station_frames[0][2]
	);
}

// in total we sent 80 frames: 16 frames with Station Name and 64 with Radiotext
void ProcessRDS(void)
{
	// first 16 frames - Station Name
	if (frame_counter < 16) {
		i2c_send_word(i2cbus, station_frames[frame_counter][0],
			station_frames[frame_counter][1],
			station_frames[frame_counter][2]
		);
	}
	else { // 16 to 79 - Radio Text frames
		uint8_t i;
		if ((frame_counter & 0x03) == 0) {
			text_frames[1][2] = 0x20 | radiotext_counter;
			i = radiotext_counter << 2; // *4
			text_frames[2][1] = radiotext_text[i];
			text_frames[2][2] = radiotext_text[i+1];
			text_frames[3][1] = radiotext_text[i+2];
			text_frames[3][2] = radiotext_text[i+3];
			radiotext_counter = (radiotext_counter + 1) % 16;
		}
		i = frame_counter & 0x03;
		i2c_send_word(i2cbus, text_frames[i][0],
			text_frames[i][1],
			text_frames[i][2]);
	}

	frame_counter = (frame_counter + 1) % 80;
	return;
}
