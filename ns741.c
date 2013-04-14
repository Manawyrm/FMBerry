#include "ns741.h"

/*
FMBerry - an cheap and easy way of transmitting music with your Pi.
Written 2013 by Tobias Mädel (t.maedel@alfeld.de)

Thanks to Rsoft for helping me preserving my sanity in spite of my non-existant knowledge of C (writing of Makefile and C-Headers)
Thanks to Paul Griffiths (1999) for his TCP socket helper.
*/

//                        "----------------------------64----------------------------------"
char radiotext_text[64] = "Hallo! Besucht tbspace.de                                       ";

/* 4 RDS 0A Groups containing Program service name */
uint8_t station_frames[20][3] =
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

	// Radiotext
/* 1 RDS 2A Group containing Radiotext chars */
	{0x03, 0xE0, 0x00},
	{0x05, 0x21, 0x20},
	{0x05, ' ' , ' ' },	// 1  2
	{0x05, ' ' , ' ' },	// 3  4

};

void ns741_rds_set_radiotext(char *text)
{
	uint8_t i = 0, isend = 0;
	for (i=0; i<64; i++)
	{
		if (text[i]==0)
			isend = 1;
		
		if (!isend)
			radiotext_text[i] = text[i];
		else
			radiotext_text[i] = ' ';
	}
	
	//strcpy( radiotext_text, text );
}

void ns741_rds_set_progname(char *text)
{
	station_frames[3][1] = text[0];
	station_frames[3][2] = text[1];
	station_frames[7][1] = text[2];
	station_frames[7][2] = text[3];
	station_frames[11][1]= text[4];
	station_frames[11][2]= text[5];
	station_frames[15][1]= text[6];
	station_frames[15][2]= text[7];
}

int ns741_rds_start(void)
{
	return TWI_send_word(station_frames[0][0],
			      station_frames[0][1],
			      station_frames[0][2]
			     );


}
void ns741_set_frequency(uint32_t f_khz)
{
	TWI_init();
	/* calculate frequency in 8.192kHz steps*/
	uint16_t val = (uint16_t)((uint32_t)f_khz*1000ULL/8192ULL);
	TWI_send(0x0A,		val);
	TWI_send(0x0B, val >> 8);
	//printf("\nSet frequency to  %lu(%lu) kHz \n", f_khz, val*8192UL/1000UL);
	return;
}

void ns741_power(uint8_t on)
{
	TWI_init();
	if (on == 1 )
		TWI_send(0x00, 0x03);	// module active
	else
		TWI_send(0x00, 0x02);	// modul deactivate

	return;
}

/************************************************************************ /
 * Write initial Data to NS741
 *
 * Thanks to Silvan König for init-sequence
 *************************************************************************/
int ns741_init(void)
{
	TWI_init();
	
	if (TWI_send(0x00, 0x00) == -1)
	{
		return -1;
	}

	TWI_writeInitData();

	TWI_send(0x02, 0x0b);
	TWI_send(0x15, 0x11);
	TWI_send(0x10, 0xE0);

	// Configuration
	TWI_send(0x02, 0x0B);
	TWI_send(0x07, 0x7E);
	TWI_send(0x08, 0x0E);
	TWI_send(0x02, 0xCA);	//0x0A Sendeleistung
	TWI_send(0x01, 0x81);

	ns741_set_frequency(99000);

	//enable transmission
	ns741_power(1);
	
	//Initialize RPI lib for RDS interrupt pin
	if (!bcm2835_init())
		return 1;
	//set RDS interrupt pin as input
	bcm2835_gpio_fsel(RDSINT, BCM2835_GPIO_FSEL_INPT);
	
	return 0;
}

void RDSINT_vect()
{
	static uint8_t frame_counter = 0;
	static uint8_t radiotext_counter = 0;
	uint8_t i = 0;
	
	if (frame_counter == 0)
	{
		//Text segment address code
		station_frames[17][2] = 0x20 | radiotext_counter;
		i = radiotext_counter<<2; // *4
		station_frames[18][1] = radiotext_text[i];
		station_frames[18][2] = radiotext_text[i+1];
		station_frames[19][1] = radiotext_text[i+2];
		station_frames[19][2] = radiotext_text[i+3];
		
		radiotext_counter = (radiotext_counter + 1) % 16;
	}
	/* Send next i2c-Frame */
	TWI_send_word(station_frames[frame_counter][0],
						  station_frames[frame_counter][1],
					      station_frames[frame_counter][2]
					      );
	/* only 4+1 RDS-groups a 4+1 i2c-frames*/
	frame_counter = (frame_counter + 1) % 20;
	return;
}