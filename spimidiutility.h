/*
 * Copyright (c) 2015-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _SPIMIDI_H
#define _SPIMIDI_H

#define MIDI_CODE_MASK  0xf0
#define MIDI_CHN_MASK   0x0f
//#define MIDI_REALTIME   0xf8
//  #define MIDI_CHAN_MODE  0xfa 
#define MIDI_OFF_NOTE   0x80
#define MIDI_ON_NOTE    0x90
#define MIDI_POLY_TOUCH 0xa0
#define MIDI_CTRL       0xb0
#define MIDI_CH_PROGRAM 0xc0
#define MIDI_TOUCH      0xd0
#define MIDI_BEND       0xe0

#define MIDI_SYSEX      0xf0
#define MIDI_Q_FRAME	0xf1
#define MIDI_SONG_POINTER 0xf2
#define MIDI_SONG_SELECT 0xf3
#define MIDI_TUNE_REQ	0xf6
#define MIDI_EOX        0xf7
#define MIDI_TIME_CLOCK 0xf8
#define MIDI_START      0xfa
#define MIDI_CONTINUE	0xfb
#define MIDI_STOP       0xfc
#define MIDI_ACTIVE_SENSING 0xfe
#define MIDI_SYS_RESET  0xff

#define MIDI_ALL_SOUND_OFF 0x78
#define MIDI_RESET_CONTROLLERS 0x79
#define MIDI_LOCAL	0x7a
#define MIDI_ALL_OFF	0x7b
#define MIDI_OMNI_OFF	0x7c
#define MIDI_OMNI_ON	0x7d
#define MIDI_MONO_ON	0x7e
#define MIDI_POLY_ON	0x7f


class CSpiMidiUtility
{
	//member variables
public:
	bool in_sysex;   // we are reading a sysex message 
	//bool done = false;       // when true, exit 
	bool notes;       // show notes? 
	bool controls;    // show continuous controllers 
	bool bender;      // record pitch bend etc.? 
	//bool excldata = true;    // record system exclusive data? 
	bool verbose;     // show text representation? 
	bool realdata;    // record real time messages? 
	bool clksencnt;   // clock and active sense count on 
	bool chmode;      // show channel mode messages 
	bool pgchanges;   // show program changes 
	//bool flush = false;	    // flush all pending MIDI data 

	uint32_t filter;            // remember state of midi filter 

	uint32_t clockcount;        // count of clocks 
	uint32_t actsensecount;     // cout of active sensing bytes 
	uint32_t notescount;        // #notes since last request 
	uint32_t notestotal;        // total #notes 
private:

	char val_format[1024]; //= "    Val %d\n";

	CHAR pCHAR[1024];
	WCHAR pWCHAR[1024];
	char nib_to_hex[17]; // = "0123456789ABCDEF";
	char vel_format[12]; // = "    Vel %d\n";

	//member functions
public:
	CSpiMidiUtility();
	~CSpiMidiUtility();
	void output(PmMessage data);
private:
	int put_pitch(int p);
	void showbytes(PmMessage data, int len, bool newline);
};
#endif //_SPIMIDI_H