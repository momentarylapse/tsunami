/*
 * Clef.cpp
 *
 *  Created on: 11.03.2016
 *      Author: michi
 */

#include "Clef.h"
#include "Scale.h"
#include "MidiData.h"

const Clef Clef::TREBLE(Clef::TYPE_TREBLE, "𝄞", 5*7-5); // \uD834\uDD1E   "\u1d11e";
const Clef Clef::TREBLE_8(Clef::TYPE_TREBLE_8, "𝄠", 4*7-5); // \uD834\uDD20   "\u1d120";
const Clef Clef::BASS(Clef::TYPE_BASS, "𝄢", 3*7-3); // \uD834\uDD22    "\u1d122";
const Clef Clef::BASS_8(Clef::TYPE_BASS_8, "𝄤", 2*7-3); // \uD834\uDD24   "\u1d124";
const Clef Clef::DRUMS(Clef::TYPE_DRUMS, "𝄥", 0);

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
		modifier = MODIFIER_NONE;
		/*if (pitch == 35)	return "bass      (akk)";
		if (pitch == 36)	return "bass";
		if (pitch == 37)	return "side stick";
		if (pitch == 38)	return "snare";
		if (pitch == 39)	return "clap";
		if (pitch == 40)	return "snare     (electronic)";
		if (pitch == 41)	return "tom - floor low";
		if (pitch == 42)	return "hihat - closed";
		if (pitch == 43)	return "tom - floor hi";
		if (pitch == 44)	return "hihat - pedal";
		if (pitch == 45)	return "tom - low";
		if (pitch == 46)	return "hihat - open";
		if (pitch == 47)	return "tom - low mid";
		if (pitch == 48)	return "tom - hi mid";
		if (pitch == 49)	return "crash 1";
		if (pitch == 50)	return "tom - hi";
		if (pitch == 51)	return "ride 1";
		if (pitch == 52)	return "chinese";
		if (pitch == 53)	return "bell ride";
		if (pitch == 54)	return "tambourine";
		if (pitch == 55)	return "splash";
		if (pitch == 56)	return "cowbell";
		if (pitch == 57)	return "crash 2";
		if (pitch == 58)	return "vibraslash?";
		if (pitch == 59)	return "ride 2";
		if (pitch == 60)	return "bongo - hi";
		if (pitch == 61)	return "bongo - low";*/
		if ((pitch == 35) or (pitch == 36)) // bass
			return 1;
		if ((pitch == 38) or (pitch == 40)) // snare
			return 5;
		if ((pitch == 48) or (pitch == 50)) // tom hi
			return 9;
		if ((pitch == 45) or (pitch == 47)) // tom mid
			return 8;
		if ((pitch == 41) or (pitch == 43)) // tom floor
			return 3;
		modifier = MODIFIER_SHARP;
		return 0;
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
	return s.transform_out(pos + offset, mod);
}
