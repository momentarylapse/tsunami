/*
 * Instrument.cpp
 *
 *  Created on: 06.12.2015
 *      Author: michi
 */

#include "../lib/hui/hui.h"
#include "Instrument.h"
#include "MidiData.h"
#include "Clef.h"

Instrument::Instrument()
{
	type = TYPE_NONE;
}

Instrument::Instrument(int _type)
{
	type = _type;
	string_pitch = default_tuning();
}

Array<int> Instrument::default_tuning() const
{
	Array<int> tuning;
	if ((type == TYPE_GUITAR) or (type == TYPE_ELECTRIC_GUITAR)){
		tuning.add(40);
		tuning.add(45);
		tuning.add(50);
		tuning.add(55);
		tuning.add(59);
		tuning.add(64);
	}
	if (type == TYPE_ELECTRIC_BASS){
		tuning.add(28);
		tuning.add(33);
		tuning.add(38);
		tuning.add(43);
	}
	if (type == TYPE_DOUBLE_BASS){
		tuning.add(28);
		tuning.add(33);
		tuning.add(38);
		tuning.add(43);
	}
	if (type == TYPE_CELLO){
		tuning.add(36);
		tuning.add(43);
		tuning.add(50);
		tuning.add(57);
	}
	if (type == TYPE_VIOLIN){
		tuning.add(55);
		tuning.add(62);
		tuning.add(69);
		tuning.add(76);
	}
	return tuning;
}

string Instrument::name() const
{
	if (type == TYPE_NONE)
		return _("  - none -");
	if (type == TYPE_PIANO)
		return _("Piano");
	if (type == TYPE_ORGAN)
		return _("Organ");
	if (type == TYPE_HAPSICHORD)
		return _("Hapsichord");
	if (type == TYPE_KEYBOARD)
		return _("Keyboard");
	if (type == TYPE_GUITAR)
		return _("Guitar");
	if (type == TYPE_ELECTRIC_GUITAR)
		return _("Electric Guitar");
	if (type == TYPE_ELECTRIC_BASS)
		return _("Electric Bass");
	if (type == TYPE_DRUMS)
		return _("Drums");
	if (type == TYPE_VOCALS)
		return _("Vocals");
	if (type == TYPE_VIOLIN)
		return _("Violin");
	if (type == TYPE_CELLO)
		return _("Cello");
	if (type == TYPE_DOUBLE_BASS)
		return _("Double Bass");
	if (type == TYPE_FLUTE)
		return _("Flute");
	if (type == TYPE_TRUMPET)
		return _("Trumpet");
	if (type == TYPE_TROMBONE)
		return _("Trombone");
	if (type == TYPE_TUBA)
		return _("Tuba");
	if (type == TYPE_HORN)
		return _("Horn");
	if (type == TYPE_SAXOPHONE)
		return _("Saxophone");
	if (type == TYPE_CLARINET)
		return _("Clarinette");
	return "???";
}

int Instrument::midi_no() const
{
	if (type == TYPE_PIANO)
		return 1;
	if (type == TYPE_HAPSICHORD)
		return 7;
	if (type == TYPE_ORGAN)
		return 20;
	if (type == TYPE_GUITAR)
		return 25;
	if (type == TYPE_ELECTRIC_GUITAR)
		return 30;
	if (type == TYPE_ELECTRIC_BASS)
		return 34;
	if (type == TYPE_VIOLIN)
		return 41;
	if (type == TYPE_CELLO)
		return 43;
	if (type == TYPE_DOUBLE_BASS)
		return 44;
	if (type == TYPE_TRUMPET)
		return 57;
	if (type == TYPE_TROMBONE)
		return 58;
	if (type == TYPE_TUBA)
		return 59;
	if (type == TYPE_HORN)
		return 61;
	if (type == TYPE_SAXOPHONE)
		return 67;
	if (type == TYPE_CLARINET)
		return 72;
	if (type == TYPE_FLUTE)
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
		type = TYPE_PIANO;
	else if ((no >= 7) and (no <= 8))
		type = TYPE_HAPSICHORD;
	else if ((no >= 17) and (no <= 24))
		type = TYPE_ORGAN;
	else if ((no >= 25) and (no <= 26))
		type = TYPE_GUITAR;
	else if ((no >= 27) and (no <= 32))
		type = TYPE_ELECTRIC_GUITAR;
	else if ((no >= 33) and (no <= 40))
		type = TYPE_ELECTRIC_BASS;
	else if (no == 41)
		type = TYPE_VIOLIN;
	else if (no == 43)
		type = TYPE_CELLO;
	else if (no == 44)
		type = TYPE_DOUBLE_BASS;
	else if (no == 57)
		type = TYPE_TRUMPET;
	else if (no == 58)
		type = TYPE_TROMBONE;
	else if (no == 59)
		type = TYPE_TUBA;
	else if ((no == 61) or (no == 70))
		type = TYPE_HORN;
	else if ((no >= 65) and (no <= 68))
		type = TYPE_SAXOPHONE;
	else if (no == 72)
		type = TYPE_CLARINET;
	else
		type = TYPE_NONE;
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
	if (type == TYPE_DRUMS)
		return Clef::DRUMS;
	if ((type == TYPE_ELECTRIC_GUITAR) or (type == TYPE_GUITAR))
		return Clef::TREBLE_8;
	if (type == TYPE_CELLO)
		return Clef::BASS;
	if ((type == TYPE_ELECTRIC_BASS) or (type == TYPE_DOUBLE_BASS))
		return Clef::BASS_8;
	return Clef::TREBLE;
}

bool Instrument::operator==(const Instrument &i) const
{
	return type == i.type;
}
