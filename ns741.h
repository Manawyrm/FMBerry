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

// Set following defines according to your region FM band
#define NS741_MIN_FREQ   8750 //  87.50 MHz
#define NS741_MAX_FREQ  10800 // 108.00 MHz
#define NS741_FREQ_STEP     5 //    .05 MHz

int  ns741_init(uint8_t i2c_bus, uint32_t f_khz);
void ns741_set_frequency(uint32_t f_khz);
void ns741_power(uint8_t on); // radio on/off
void ns741_mute(uint8_t on); // mute on/off
// strength 0 to 3, corresponding output: 0.5mW, 0.8mW, 1.0mW, 2.0mW
void ns741_txpwr(uint8_t strength);
void ns741_volume(uint8_t gain); // output gain 0-6, or -9dB to +9db, 3dB step
void ns741_input_gain(uint8_t on); // input audio gain -9dB on/off
void ns741_stereo(uint8_t on); // stereo on/off

void ns741_rds(uint8_t on); // RDS on/off
void ns741_rds_cp(uint8_t cp); // RDS_CP flag, third block type: C (cp=0) or C' (cp=1)
void ns741_rds_set_progname(const char *text);
void ns741_rds_set_rds_pi(uint16_t rdspi);
void ns741_rds_set_rds_pty(uint8_t rdspty);
void ns741_rds_set_radiotext(const char *text);
void ns741_rds_reset_radiotext(void); // toggle Reset flag

void ns741_rds_debug(uint8_t on);

// RDSINT handler
uint8_t ns741_rds_isr(void);
#endif
