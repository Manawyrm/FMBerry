#ifndef __NS741_H_FMBERRY_
#define __NS741_H_FMBERRY_

#include "i2c.h"

int  ns741_init(void);
void ns741_set_frequency(uint32_t f_khz);
void ns741_power(uint8_t on);
void ns741_mute(uint8_t on);
void ns741_txpwr(uint8_t strength);

int  ns741_rds_start(void);
void ns741_rds_set_radiotext(const char *text);
void ns741_rds_set_progname(const char *text);

void ProcessRDS(void);
#endif
