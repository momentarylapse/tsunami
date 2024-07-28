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


namespace tsunami {

Instrument::Instrument() {
	type = Type::None;
}

Instrument::Instrument(Type _type) {
	type = _type;
	string_pitch = default_tuning();
}

Array<int> Instrument::default_tuning() const {
	if ((type == Type::Guitar) or (type == Type::ElectricGuitar))
		return {40,45,50,55,59,64};
	if (type == Type::ElectricBass)
		return {28,33,38,43};
	if (type == Type::DoubleBass)
		return {28,33,38,43};
	if (type == Type::Cello)
		return {36,43,50,57};
	if (type == Type::Violin)
		return {55,62,69,76};
	if (type == Type::Lute)
		return {36,38,40,45,50,55,59,64};
	return {};
}

string Instrument::name() const {
	if (type == Type::None)
		return _("  - none -");
	if (type == Type::Piano)
		return _("Piano");
	if (type == Type::Organ)
		return _("Organ");
	if (type == Type::Harpsichord)
		return _("Harpsichord");
	if (type == Type::Keyboard)
		return _("Keyboard");
	if (type == Type::Guitar)
		return _("Guitar");
	if (type == Type::ElectricGuitar)
		return _("Electric Guitar");
	if (type == Type::ElectricBass)
		return _("Electric Bass");
	if (type == Type::Drums)
		return _("Drums");
	if (type == Type::Vocals)
		return _("Vocals");
	if (type == Type::Violin)
		return _("Violin");
	if (type == Type::Cello)
		return _("Cello");
	if (type == Type::DoubleBass)
		return _("Double Bass");
	if (type == Type::Flute)
		return _("Flute");
	if (type == Type::Trumpet)
		return _("Trumpet");
	if (type == Type::Trombone)
		return _("Trombone");
	if (type == Type::Tuba)
		return _("Tuba");
	if (type == Type::Horn)
		return _("Horn");
	if (type == Type::Saxophone)
		return _("Saxophone");
	if (type == Type::Clarinet)
		return _("Clarinet");
	if (type == Type::Lute)
		return _("Lute");
	return "???";
}

int Instrument::midi_no() const {
	if (type == Type::Piano)
		return 1;
	if (type == Type::Harpsichord)
		return 7;
	if (type == Type::Organ)
		return 20;
	if (type == Type::Guitar)
		return 25;
	if (type == Type::ElectricGuitar)
		return 30;
	if (type == Type::ElectricBass)
		return 34;
	if (type == Type::Violin)
		return 41;
	if (type == Type::Cello)
		return 43;
	if (type == Type::DoubleBass)
		return 44;
	if (type == Type::Trumpet)
		return 57;
	if (type == Type::Trombone)
		return 58;
	if (type == Type::Tuba)
		return 59;
	if (type == Type::Horn)
		return 61;
	if (type == Type::Saxophone)
		return 67;
	if (type == Type::Clarinet)
		return 72;
	if (type == Type::Flute)
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
		type = Type::Piano;
	else if ((no >= 7) and (no <= 8))
		type = Type::Harpsichord;
	else if ((no >= 17) and (no <= 24))
		type = Type::Organ;
	else if ((no >= 25) and (no <= 26))
		type = Type::Guitar;
	else if ((no >= 27) and (no <= 32))
		type = Type::ElectricGuitar;
	else if ((no >= 33) and (no <= 40))
		type = Type::ElectricBass;
	else if (no == 41)
		type = Type::Violin;
	else if (no == 43)
		type = Type::Cello;
	else if (no == 44)
		type = Type::DoubleBass;
	else if (no == 57)
		type = Type::Trumpet;
	else if (no == 58)
		type = Type::Trombone;
	else if (no == 59)
		type = Type::Tuba;
	else if ((no == 61) or (no == 70))
		type = Type::Horn;
	else if ((no >= 65) and (no <= 68))
		type = Type::Saxophone;
	else if (no == 72)
		type = Type::Clarinet;
	else
		type = Type::None;
	string_pitch = default_tuning();
}

Array<Instrument> Instrument::enumerate() {
	Array<Instrument> instruments;
	for (int i=0; i<(int)Instrument::Type::Count; i++)
		instruments.add(Instrument((Instrument::Type)i));
	return instruments;
}

const Clef& Instrument::get_clef() const {
	if (type == Type::Drums)
		return Clef::Drums;
	if ((type == Type::ElectricGuitar) or (type == Type::Guitar) or (type == Type::Lute))
		return Clef::Treble8;
	if (type == Type::Cello)
		return Clef::Bass;
	if ((type == Type::ElectricBass) or (type == Type::DoubleBass))
		return Clef::Bass8;
	return Clef::Treble;
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

}
