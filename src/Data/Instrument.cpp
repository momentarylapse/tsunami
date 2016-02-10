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
	return "???";
}

int Instrument::midi_no() const
{
	return 1;
}

bool Instrument::is_default_tuning(const Array<int>& tuning) const
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
	type = TYPE_NONE;
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
