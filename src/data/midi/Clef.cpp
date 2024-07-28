/*
 * Clef.cpp
 *
 *  Created on: 11.03.2016
 *      Author: michi
 */

#include "Clef.h"
#include "Scale.h"
#include "MidiData.h"


namespace tsunami {

const Clef Clef::Treble(ClefType::Treble, u8"\U0001d11e", 5*7-5);
const Clef Clef::Treble8(ClefType::Treble8, u8"\U0001d120", 4*7-5);
const Clef Clef::Bass(ClefType::Bass, u8"\U0001d122", 3*7-3);
const Clef Clef::Bass8(ClefType::Bass8, u8"\U0001d124", 2*7-3);
const Clef Clef::Drums(ClefType::Drums, u8"\U0001d125", 0);

struct DrumClefPosition {
	int pitch, clef_pos;
	NoteModifier modifier;
};
static constexpr int NumDrumClefPositions = 19;
const DrumClefPosition DrumClefPositions[NumDrumClefPositions] = {
	// drums
	{DrumPitch::Bass, 1, NoteModifier::None}, // "F"
	{DrumPitch::BassAccoustic, 1, NoteModifier::None},
	{DrumPitch::TomFloorLow, 3, NoteModifier::None}, // "A"
	{DrumPitch::TomFloorHi, 3, NoteModifier::None},
	{DrumPitch::TomLow, 6, NoteModifier::None}, // "D"
	{DrumPitch::TomLowMid, 6, NoteModifier::None},
	{DrumPitch::TomHiMid, 7, NoteModifier::None}, // "E"
	{DrumPitch::TomHi, 7, NoteModifier::None},
	{DrumPitch::Snare, 5, NoteModifier::None}, // "C"
	{DrumPitch::SnareElectronic, 5, NoteModifier::None},

	// cymbals
	{DrumPitch::HihatPedal, -1, NoteModifier::None}, // "D"
	{DrumPitch::HihatOpen, 9, NoteModifier::Sharp}, // "G"
	{DrumPitch::HihatClosed, 9, NoteModifier::Flat},
	{DrumPitch::Ride1, 8, NoteModifier::None}, // "F"
	{DrumPitch::Ride2, 8, NoteModifier::None},
	{DrumPitch::BellRide, 8, NoteModifier::Flat},
	{DrumPitch::Crash1, 10, NoteModifier::None}, // "A"
	{DrumPitch::Crash2, 11, NoteModifier::None}, // "B"
	{DrumPitch::Chinese, 13, NoteModifier::None}, // "D"
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


Clef::Clef(ClefType _type, const string &_symbol, int _offset) {
	type = _type;
	symbol = _symbol;
	offset = _offset;
}

// TODO account for scale!
int Clef::pitch_to_position(int pitch, const Scale &s, NoteModifier &modifier) const {
	if (type == ClefType::Drums) {
		for (int i=0; i<NumDrumClefPositions; i++)
			if (pitch == DrumClefPositions[i].pitch) {
				modifier = DrumClefPositions[i].modifier;
				return DrumClefPositions[i].clef_pos;
			}

		modifier = NoteModifier::Sharp;
		return 13;
	}
	int octave = pitch_get_octave(pitch);
	int rel = pitch_to_rel(pitch);

	const int pp[12] = {0,0,1,2,2,3,3,4,4,5,6,6};
	const NoteModifier ss[12] = {NoteModifier::None,NoteModifier::Sharp,NoteModifier::None,NoteModifier::Flat,NoteModifier::None,NoteModifier::None,NoteModifier::Sharp,NoteModifier::None,NoteModifier::Sharp,NoteModifier::None,NoteModifier::Flat,NoteModifier::None};

	modifier = ss[rel];
	return pp[rel] + 7 * octave - offset;
}

int Clef::position_to_pitch(int pos, const Scale &s, NoteModifier mod) const {
	if (type == ClefType::Drums){
		for (int i=0; i<NumDrumClefPositions; i++)
			if ((pos == DrumClefPositions[i].clef_pos) and (mod == DrumClefPositions[i].modifier))
				return DrumClefPositions[i].pitch;
		return 70;//DrumPitch::BASS;
	}
	return s.transform_out(position_to_uniclef(pos), mod);
}

int Clef::position_to_uniclef(int pos) const {
	return pos + offset;
}

}
