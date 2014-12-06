/*
    FMBerry - an cheap and easy way of transmitting music with your Pi.

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
#ifndef __NS741_H_FMBERRY_
#define __NS741_H_FMBERRY_

#include <stdint.h>

int  ns741_init(uint8_t i2c_bus, uint32_t f_khz);
void ns741_set_frequency(uint32_t f_khz);
void ns741_power(uint8_t on);
void ns741_mute(uint8_t on);
void ns741_txpwr(uint8_t strength);
void ns741_stereo(uint8_t on);
void ns741_volume(uint8_t gain);
void ns741_input_gain(uint8_t on);

void ns741_rds(uint8_t on);
int  ns741_rds_start(void);
void ns741_rds_set_radiotext(const char *text);
void ns741_rds_set_progname(const char *text);

void ProcessRDS(void);
#endif
