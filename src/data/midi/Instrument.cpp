/*
 * Instrument.cpp
 *
 *  Created on: 06.12.2015
 *      Author: michi
 */

#include "Instrument.h"
#include "MidiData.h"
#include "Clef.h"
#include "../../lib/hui/hui.h"

Instrument::Instrument() {
	type = Type::NONE;
}

Instrument::Instrument(Type _type) {
	type = _type;
	string_pitch = default_tuning();
}

Array<int> Instrument::default_tuning() const {
	if ((type == Type::GUITAR) or (type == Type::ELECTRIC_GUITAR))
		return {40,45,50,55,59,64};
	if (type == Type::ELECTRIC_BASS)
		return {28,33,38,43};
	if (type == Type::DOUBLE_BASS)
		return {28,33,38,43};
	if (type == Type::CELLO)
		return {36,43,50,57};
	if (type == Type::VIOLIN)
		return {55,62,69,76};
	if (type == Type::LUTE)
		return {36,38,40,45,50,55,59,64};
	return {};
}

string Instrument::name() const {
	if (type == Type::NONE)
		return _("  - none -");
	if (type == Type::PIANO)
		return _("Piano");
	if (type == Type::ORGAN)
		return _("Organ");
	if (type == Type::HAPSICHORD)
		return _("Harpsichord");
	if (type == Type::KEYBOARD)
		return _("Keyboard");
	if (type == Type::GUITAR)
		return _("Guitar");
	if (type == Type::ELECTRIC_GUITAR)
		return _("Electric Guitar");
	if (type == Type::ELECTRIC_BASS)
		return _("Electric Bass");
	if (type == Type::DRUMS)
		return _("Drums");
	if (type == Type::VOCALS)
		return _("Vocals");
	if (type == Type::VIOLIN)
		return _("Violin");
	if (type == Type::CELLO)
		return _("Cello");
	if (type == Type::DOUBLE_BASS)
		return _("Double Bass");
	if (type == Type::FLUTE)
		return _("Flute");
	if (type == Type::TRUMPET)
		return _("Trumpet");
	if (type == Type::TROMBONE)
		return _("Trombone");
	if (type == Type::TUBA)
		return _("Tuba");
	if (type == Type::HORN)
		return _("Horn");
	if (type == Type::SAXOPHONE)
		return _("Saxophone");
	if (type == Type::CLARINET)
		return _("Clarinet");
	if (type == Type::LUTE)
		return _("Lute");
	return "???";
}

int Instrument::midi_no() const {
	if (type == Type::PIANO)
		return 1;
	if (type == Type::HAPSICHORD)
		return 7;
	if (type == Type::ORGAN)
		return 20;
	if (type == Type::GUITAR)
		return 25;
	if (type == Type::ELECTRIC_GUITAR)
		return 30;
	if (type == Type::ELECTRIC_BASS)
		return 34;
	if (type == Type::VIOLIN)
		return 41;
	if (type == Type::CELLO)
		return 43;
	if (type == Type::DOUBLE_BASS)
		return 44;
	if (type == Type::TRUMPET)
		return 57;
	if (type == Type::TROMBONE)
		return 58;
	if (type == Type::TUBA)
		return 59;
	if (type == Type::HORN)
		return 61;
	if (type == Type::SAXOPHONE)
		return 67;
	if (type == Type::CLARINET)
		return 72;
	if (type == Type::FLUTE)
		return 74;
	return 1;
}

bool Instrument::has_default_tuning() const {
	Array<int> def = default_tuning();
	if (def.num != string_pitch.num)
		return false;
	for (int i=0; i<string_pitch.num; i++)
		if (def[i] != string_pitch[i])
			return false;
	return true;
}

void Instrument::set_midi_no(int no) {
	if ((no >= 1) and (no <= 6))
		type = Type::PIANO;
	else if ((no >= 7) and (no <= 8))
		type = Type::HAPSICHORD;
	else if ((no >= 17) and (no <= 24))
		type = Type::ORGAN;
	else if ((no >= 25) and (no <= 26))
		type = Type::GUITAR;
	else if ((no >= 27) and (no <= 32))
		type = Type::ELECTRIC_GUITAR;
	else if ((no >= 33) and (no <= 40))
		type = Type::ELECTRIC_BASS;
	else if (no == 41)
		type = Type::VIOLIN;
	else if (no == 43)
		type = Type::CELLO;
	else if (no == 44)
		type = Type::DOUBLE_BASS;
	else if (no == 57)
		type = Type::TRUMPET;
	else if (no == 58)
		type = Type::TROMBONE;
	else if (no == 59)
		type = Type::TUBA;
	else if ((no == 61) or (no == 70))
		type = Type::HORN;
	else if ((no >= 65) and (no <= 68))
		type = Type::SAXOPHONE;
	else if (no == 72)
		type = Type::CLARINET;
	else
		type = Type::NONE;
	string_pitch = default_tuning();
}

Array<Instrument> Instrument::enumerate() {
	Array<Instrument> instruments;
	for (int i=0; i<(int)Instrument::Type::NUM_TYPES; i++)
		instruments.add(Instrument((Instrument::Type)i));
	return instruments;
}

const Clef& Instrument::get_clef() const {
	if (type == Type::DRUMS)
		return Clef::_DRUMS;
	if ((type == Type::ELECTRIC_GUITAR) or (type == Type::GUITAR) or (type == Type::LUTE))
		return Clef::_TREBLE_8;
	if (type == Type::CELLO)
		return Clef::_BASS;
	if ((type == Type::ELECTRIC_BASS) or (type == Type::DOUBLE_BASS))
		return Clef::_BASS_8;
	return Clef::_TREBLE;
}

bool Instrument::has_strings() const {
	return string_pitch.num > 0;
}

bool Instrument::operator==(const Instrument &i) const {
	return type == i.type;
}

bool Instrument::operator!=(const Instrument &i) const {
	return !(*this == i);
}

int Instrument::highest_usable_string(int pitch) const {
	if (!has_strings())
		return -1;
	int n = 0;
	for (int i=0; i<string_pitch.num; i++)
		if (pitch >= string_pitch[i])
			n = i;
	return n;
}
int Instrument::make_string_valid(int pitch, int stringno) const {
	if (!has_strings())
		return -1;
	int hus = highest_usable_string(pitch);
	if (stringno < 0 or stringno >= hus)
		return hus;
	return stringno;
}
