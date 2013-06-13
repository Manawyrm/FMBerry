#ifndef __NS741_H_FMBERRY_
#define __NS741_H_FMBERRY_

#include "i2c.h"
#include <bcm2835.h>
#define RDSINT RPI_V2_GPIO_P1_11

void ns741_set_frequency(uint32_t f_khz);
void ns741_power(uint8_t on);
int ns741_init(void);
void ns741_rds_set_radiotext(char *text);
void ns741_rds_set_progname(char *text);
int ns741_rds_start(void);
void RDSINT_vect();
void ns741_mute(uint8_t on);
void ns741_txpwr(uint8_t strength);

#endif
