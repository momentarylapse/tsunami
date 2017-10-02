/*
 * Clef.cpp
 *
 *  Created on: 11.03.2016
 *      Author: michi
 */

#include "Clef.h"
#include "Scale.h"
#include "MidiData.h"

const Clef Clef::TREBLE(Clef::TYPE_TREBLE, "\U0001d11e", 5*7-5); // U+1d11e "𝄞"
const Clef Clef::TREBLE_8(Clef::TYPE_TREBLE_8, "\U0001d120", 4*7-5); // U+1d120 "𝄠"
const Clef Clef::BASS(Clef::TYPE_BASS, "\U0001d122", 3*7-3); // U+1d122 "𝄢"
const Clef Clef::BASS_8(Clef::TYPE_BASS_8, "\U0001d124", 2*7-3); // U+1d124 "𝄤"
const Clef Clef::DRUMS(Clef::TYPE_DRUMS, "\U0001d125", 0); // U+1d125 "𝄥"

struct DrumClefPosition
{
	int pitch, clef_pos, modifier;
};
const int NUM_DRUM_CLEF_POSITIONS = 19;
const DrumClefPosition DrumClefPositions[NUM_DRUM_CLEF_POSITIONS] =
{
	// drums
	{DRUM_PITCH_BASS, 1, MODIFIER_NONE},
	{DRUM_PITCH_BASS_ACCOUSTIC, 1, MODIFIER_NONE},
	{DRUM_PITCH_TOM_FLOOR_LOW, 2, MODIFIER_NONE},
	{DRUM_PITCH_TOM_FLOOR_HI, 3, MODIFIER_NONE},
	{DRUM_PITCH_TOM_LOW, 4, MODIFIER_NONE},
	{DRUM_PITCH_SNARE, 5, MODIFIER_NONE},
	{DRUM_PITCH_SNARE_ELECTRONIC, 5, MODIFIER_NONE},
	{DRUM_PITCH_TOM_LOW_MID, 6, MODIFIER_NONE},
	{DRUM_PITCH_TOM_HI_MID, 7, MODIFIER_NONE},
	{DRUM_PITCH_TOM_HI, 8, MODIFIER_NONE},

	// cymbals
	{DRUM_PITCH_HIHAT_PEDAL, -1, MODIFIER_SHARP},
	{DRUM_PITCH_HIHAT_OPEN, 7, MODIFIER_SHARP},
	{DRUM_PITCH_HIHAT_CLOSED, 7, MODIFIER_FLAT},
	{DRUM_PITCH_RIDE_1, 8, MODIFIER_SHARP},
	{DRUM_PITCH_RIDE_2, 8, MODIFIER_SHARP},
	{DRUM_PITCH_BELL_RIDE, 8, MODIFIER_FLAT},
	{DRUM_PITCH_CRASH_1, 9, MODIFIER_SHARP},
	{DRUM_PITCH_CRASH_2, 9, MODIFIER_SHARP},
	{DRUM_PITCH_CHINESE, 9, MODIFIER_FLAT},
};

/*
		DRUM_PITCH_SIDE_STICK = 37,
		DRUM_PITCH_CLAP = 39,
		DRUM_PITCH_TAMBOURINE = 54,
		DRUM_PITCH_SPLASH = 55,
		DRUM_PITCH_COWBELL = 56,
		DRUM_PITCH_VIBRASLASH = 58,
		DRUM_PITCH_BONGO_HI = 60,
		DRUM_PITCH_BONGO_LOW = 61,
*/


Clef::Clef(int _type, const string &_symbol, int _offset)
{
	type = _type;
	symbol = _symbol;
	offset = _offset;
}

// TODO account for scale!
int Clef::pitch_to_position(int pitch, const Scale &s, int &modifier) const
{
	if (type == TYPE_DRUMS){
		for (int i=0; i<NUM_DRUM_CLEF_POSITIONS; i++)
			if (pitch == DrumClefPositions[i].pitch){
				modifier = DrumClefPositions[i].modifier;
				return DrumClefPositions[i].clef_pos;
			}

		modifier = MODIFIER_SHARP;
		return 13;
	}
	int octave = pitch_get_octave(pitch);
	int rel = pitch_to_rel(pitch);

	const int pp[12] = {0,0,1,2,2,3,3,4,4,5,6,6};
	const int ss[12] = {MODIFIER_NONE,MODIFIER_SHARP,MODIFIER_NONE,MODIFIER_FLAT,MODIFIER_NONE,MODIFIER_NONE,MODIFIER_SHARP,MODIFIER_NONE,MODIFIER_SHARP,MODIFIER_NONE,MODIFIER_FLAT,MODIFIER_NONE};

	modifier = ss[rel];
	return pp[rel] + 7 * octave - offset;
}

int Clef::position_to_pitch(int pos, const Scale &s, int mod) const
{
	if (type == TYPE_DRUMS){
		for (int i=0; i<NUM_DRUM_CLEF_POSITIONS; i++)
			if ((pos == DrumClefPositions[i].clef_pos) and (mod == DrumClefPositions[i].modifier))
				return DrumClefPositions[i].pitch;
		return 70;//DRUM_PITCH_BASS;
	}
	return s.transform_out(pos + offset, mod);
}
