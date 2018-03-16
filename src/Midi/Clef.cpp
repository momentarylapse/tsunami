/*
 * Clef.cpp
 *
 *  Created on: 11.03.2016
 *      Author: michi
 */

#include "Clef.h"
#include "Scale.h"
#include "MidiData.h"

const Clef Clef::_TREBLE(Clef::Type::TREBLE, "\U0001d11e", 5*7-5); // U+1d11e "𝄞"
const Clef Clef::_TREBLE_8(Clef::Type::TREBLE_8, "\U0001d120", 4*7-5); // U+1d120 "𝄠"
const Clef Clef::_BASS(Clef::Type::BASS, "\U0001d122", 3*7-3); // U+1d122 "𝄢"
const Clef Clef::_BASS_8(Clef::Type::BASS_8, "\U0001d124", 2*7-3); // U+1d124 "𝄤"
const Clef Clef::_DRUMS(Clef::Type::DRUMS, "\U0001d125", 0); // U+1d125 "𝄥"

struct DrumClefPosition
{
	int pitch, clef_pos, modifier;
};
const int NUM_DRUM_CLEF_POSITIONS = 19;
const DrumClefPosition DrumClefPositions[NUM_DRUM_CLEF_POSITIONS] =
{
	// drums
	{DrumPitch::BASS, 1, Modifier::NONE},
	{DrumPitch::BASS_ACCOUSTIC, 1, Modifier::NONE},
	{DrumPitch::TOM_FLOOR_LOW, 2, Modifier::NONE},
	{DrumPitch::TOM_FLOOR_HI, 3, Modifier::NONE},
	{DrumPitch::TOM_LOW, 4, Modifier::NONE},
	{DrumPitch::SNARE, 5, Modifier::NONE},
	{DrumPitch::SNARE_ELECTRONIC, 5, Modifier::NONE},
	{DrumPitch::TOM_LOW_MID, 6, Modifier::NONE},
	{DrumPitch::TOM_HI_MID, 7, Modifier::NONE},
	{DrumPitch::TOM_HI, 8, Modifier::NONE},

	// cymbals
	{DrumPitch::HIHAT_PEDAL, -1, Modifier::SHARP},
	{DrumPitch::HIHAT_OPEN, 7, Modifier::SHARP},
	{DrumPitch::HIHAT_CLOSED, 7, Modifier::FLAT},
	{DrumPitch::RIDE_1, 8, Modifier::SHARP},
	{DrumPitch::RIDE_2, 8, Modifier::SHARP},
	{DrumPitch::BELL_RIDE, 8, Modifier::FLAT},
	{DrumPitch::CRASH_1, 9, Modifier::SHARP},
	{DrumPitch::CRASH_2, 9, Modifier::SHARP},
	{DrumPitch::CHINESE, 9, Modifier::FLAT},
};

/*
		DrumPitch::SIDE_STICK = 37,
		DrumPitch::CLAP = 39,
		DrumPitch::TAMBOURINE = 54,
		DrumPitch::SPLASH = 55,
		DrumPitch::COWBELL = 56,
		DrumPitch::VIBRASLASH = 58,
		DrumPitch::BONGO_HI = 60,
		DrumPitch::BONGO_LOW = 61,
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
	if (type == Type::DRUMS){
		for (int i=0; i<NUM_DRUM_CLEF_POSITIONS; i++)
			if (pitch == DrumClefPositions[i].pitch){
				modifier = DrumClefPositions[i].modifier;
				return DrumClefPositions[i].clef_pos;
			}

		modifier = Modifier::SHARP;
		return 13;
	}
	int octave = pitch_get_octave(pitch);
	int rel = pitch_to_rel(pitch);

	const int pp[12] = {0,0,1,2,2,3,3,4,4,5,6,6};
	const int ss[12] = {Modifier::NONE,Modifier::SHARP,Modifier::NONE,Modifier::FLAT,Modifier::NONE,Modifier::NONE,Modifier::SHARP,Modifier::NONE,Modifier::SHARP,Modifier::NONE,Modifier::FLAT,Modifier::NONE};

	modifier = ss[rel];
	return pp[rel] + 7 * octave - offset;
}

int Clef::position_to_pitch(int pos, const Scale &s, int mod) const
{
	if (type == Type::DRUMS){
		for (int i=0; i<NUM_DRUM_CLEF_POSITIONS; i++)
			if ((pos == DrumClefPositions[i].clef_pos) and (mod == DrumClefPositions[i].modifier))
				return DrumClefPositions[i].pitch;
		return 70;//DrumPitch::BASS;
	}
	return s.transform_out(pos + offset, mod);
}
