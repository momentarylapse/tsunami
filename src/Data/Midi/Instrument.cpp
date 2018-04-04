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

Instrument::Instrument()
{
	type = Type::NONE;
}

Instrument::Instrument(int _type)
{
	type = _type;
	string_pitch = default_tuning();
}

Array<int> Instrument::default_tuning() const
{
	Array<int> tuning;
	if ((type == Type::GUITAR) or (type == Type::ELECTRIC_GUITAR)){
		tuning.add(40);
		tuning.add(45);
		tuning.add(50);
		tuning.add(55);
		tuning.add(59);
		tuning.add(64);
	}
	if (type == Type::ELECTRIC_BASS){
		tuning.add(28);
		tuning.add(33);
		tuning.add(38);
		tuning.add(43);
	}
	if (type == Type::DOUBLE_BASS){
		tuning.add(28);
		tuning.add(33);
		tuning.add(38);
		tuning.add(43);
	}
	if (type == Type::CELLO){
		tuning.add(36);
		tuning.add(43);
		tuning.add(50);
		tuning.add(57);
	}
	if (type == Type::VIOLIN){
		tuning.add(55);
		tuning.add(62);
		tuning.add(69);
		tuning.add(76);
	}
	return tuning;
}

string Instrument::name() const
{
	if (type == Type::NONE)
		return _("  - none -");
	if (type == Type::PIANO)
		return _("Piano");
	if (type == Type::ORGAN)
		return _("Organ");
	if (type == Type::HAPSICHORD)
		return _("Hapsichord");
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
		return _("Clarinette");
	return "???";
}

int Instrument::midi_no() const
{
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

bool Instrument::has_default_tuning() const
{
	Array<int> def = default_tuning();
	if (def.num != string_pitch.num)
		return false;
	for (int i=0; i<string_pitch.num; i++)
		if (def[i] != string_pitch[i])
			return false;
	return true;
}

void Instrument::set_midi_no(int no)
{
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

Array<Instrument> Instrument::enumerate()
{
	Array<Instrument> instruments;
	for (int i=0; i<NUM_TYPES; i++)
		instruments.add(Instrument(i));
	return instruments;
}

const Clef& Instrument::get_clef() const
{
	if (type == Type::DRUMS)
		return Clef::_DRUMS;
	if ((type == Type::ELECTRIC_GUITAR) or (type == Type::GUITAR))
		return Clef::_TREBLE_8;
	if (type == Type::CELLO)
		return Clef::_BASS;
	if ((type == Type::ELECTRIC_BASS) or (type == Type::DOUBLE_BASS))
		return Clef::_BASS_8;
	return Clef::_TREBLE;
}

bool Instrument::operator==(const Instrument &i) const
{
	return type == i.type;
}
