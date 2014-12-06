/*  Some RBDS standard defines used for NS741 chip on MMR-70

    Copyright (c) 2014 Andrey Chilikin (https://github.com/achilikin)
    
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

#ifndef __RDS_CONSTANTS_H__
#define __RDS_CONSTANTS_H__

// RBDS Standard, Annex D
// Program Identification: Country Code, Area Coverage, Reference Number
#define RDS_PI(CC,AC,RN) ((((uint16_t)RN) & 0xFF) | \
	((((uint16_t)AC) & 0x0F) << 8) | \
	((((uint16_t)CC) & 0x0F) << 12))

// Some European Broadcasting Area Country Codes
// It is better to use CC not for your country
// to avoid collision with any local radio stations
#define RDS_GERMANY 0x0D
#define RDS_IRELAND 0x02
#define RDS_RUSSIA  0x07
#define RDS_UK      0x0C

// Coverage-Area Codes
#define CAC_LOCAL    0 // Local, single transmitter
#define CAC_INTER    1 // International
#define CAC_NATIONAL 2 // National
#define CAC_SUPRA    3 // Supra-Regional
#define CAC_REGION   4 // Regional + Region code 0-11

#define RDS_GT(GT,VER) ((uint16_t)(((((uint8_t)GT)&0xF)<<1) | (((uint8_t)VER)&0x1)) << 11)
#define RDS_TP 0x4000 // Traffic Program ID, do not use with MMR-70
#define RDS_PTY(PTY) ((((uint16_t)PTY) & 0x1F) << 5)
#define RDS_TA  0x0010 // Traffic Announcement
#define RDS_MS  0x0008 // Music/Speech (group 0A/0B)
#define RDS_DI  0x0004 // Decoder-Identification control code (group 0A/0B)
#define RDS_AB  0x0010 // Text A/B reset flag

#define RDS_PSIM 0x0003 // Program Service name index mask
#define RDS_RTIM 0x000F // Radiotext index mask
#define RDS_PTYM (0x1F<<5)

// Some of PTY codes, for full list see RBDS Standard, Annex F
// ftp://ftp.rds.org.uk/pub/acrobat/rbds1998.pdf
// DO NOT USE 30 or 31!!!
#define PTY_NONE    0
#define PTY_NEWS    1
#define PTY_INFORM  2
#define PTY_SPORT   3
#define PTY_TALK    4
#define PTY_ROCK    5
#define PTY_COUNTRY 10
#define PTY_JAZZ    14
#define PTY_CLASSIC 15
#define PTY_WEATHER 29

#endif
