#ifndef __NS741_H_FMBERRY_
#define __NS741_H_FMBERRY_

#include <stdint.h>

int  ns741_init(uint8_t i2c_bus);
void ns741_set_frequency(uint32_t f_khz);
void ns741_power(uint8_t on);
void ns741_mute(uint8_t on);
void ns741_txpwr(uint8_t strength);
void ns741_stereo(uint8_t on);

void ns741_rds(uint8_t on);
int  ns741_rds_start(void);
void ns741_rds_set_radiotext(const char *text);
void ns741_rds_set_progname(const char *text);

void ProcessRDS(void);
#endif
