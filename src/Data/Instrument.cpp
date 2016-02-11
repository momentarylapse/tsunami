/*
 * Instrument.cpp
 *
 *  Created on: 06.12.2015
 *      Author: michi
 */

#include "../lib/hui/hui.h"
#include "Instrument.h"
#include "MidiData.h"

Instrument::Instrument()
{
	type = TYPE_NONE;
}

Instrument::Instrument(int _type)
{
	type = _type;
	tuning = default_tuning();
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
		return _("  - keins -");
	if (type == TYPE_PIANO)
		return _("Piano");
	if (type == TYPE_ORGAN)
		return _("Orgel");
	if (type == TYPE_HAPSICHORD)
		return _("Hapsichord");
	if (type == TYPE_KEYBOARD)
		return _("Keyboard");
	if (type == TYPE_GUITAR)
		return _("Gitarre");
	if (type == TYPE_ELECTRIC_GUITAR)
		return _("E-Gitarre");
	if (type == TYPE_ELECTRIC_BASS)
		return _("E-Bass");
	if (type == TYPE_DRUMS)
		return _("Schlagzeug");
	if (type == TYPE_VOCALS)
		return _("Gesang");
	if (type == TYPE_VIOLIN)
		return _("Geige");
	if (type == TYPE_CELLO)
		return _("Cello");
	if (type == TYPE_DOUBLE_BASS)
		return _("Kontra-Bass");
	if (type == TYPE_FLUTE)
		return _("Fl&ote");
	if (type == TYPE_TRUMPET)
		return _("Trompete");
	if (type == TYPE_TROMBONE)
		return _("Posaune");
	if (type == TYPE_TUBA)
		return _("Tuba");
	if (type == TYPE_HORN)
		return _("Horn");
	if (type == TYPE_SAXOPHONE)
		return _("Saxophon");
	if (type == TYPE_CLARINET)
		return _("Klarinette");
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
	if (def.num != tuning.num)
		return false;
	for (int i=0; i<tuning.num; i++)
		if (def[i] != tuning[i])
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
	tuning = default_tuning();
}

Array<Instrument> Instrument::enumerate()
{
	Array<Instrument> instruments;
	for (int i=0; i<NUM_TYPES; i++)
		instruments.add(Instrument(i));
	return instruments;
}

int Instrument::get_clef()
{
	if (type == TYPE_DRUMS)
		return CLEF_TYPE_DRUMS;
	if ((type == TYPE_ELECTRIC_GUITAR) or (type == TYPE_GUITAR))
		return CLEF_TYPE_TREBLE_8;
	if (type == TYPE_CELLO)
		return CLEF_TYPE_BASS;
	if ((type == TYPE_ELECTRIC_BASS) or (type == TYPE_DOUBLE_BASS))
		return CLEF_TYPE_BASS_8;
	return CLEF_TYPE_TREBLE;
}

bool Instrument::operator==(const Instrument &i) const
{
	return type == i.type;
}
