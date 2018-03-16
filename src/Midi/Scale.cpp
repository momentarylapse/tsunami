/*
 * Scale.cpp
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#include "../lib/hui/hui.h"
#include "Scale.h"
#include "MidiData.h"



static const int scale_major_modifiers[12][7] = {
	{Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::NONE}, // C
	{Modifier::NONE, Modifier::FLAT, Modifier::FLAT, Modifier::NONE, Modifier::FLAT, Modifier::FLAT, Modifier::FLAT}, // Db
	{Modifier::SHARP,Modifier::NONE, Modifier::NONE, Modifier::SHARP,Modifier::NONE, Modifier::NONE, Modifier::NONE}, // D
	{Modifier::NONE, Modifier::NONE, Modifier::FLAT, Modifier::NONE, Modifier::NONE, Modifier::FLAT, Modifier::FLAT}, // Eb
	{Modifier::SHARP,Modifier::SHARP,Modifier::NONE, Modifier::SHARP,Modifier::SHARP,Modifier::NONE, Modifier::NONE}, // E
	{Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::FLAT}, // F
	{Modifier::SHARP,Modifier::SHARP,Modifier::SHARP,Modifier::SHARP,Modifier::SHARP,Modifier::SHARP,Modifier::NONE}, // F#
	{Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::SHARP,Modifier::NONE, Modifier::NONE, Modifier::NONE}, // G
	{Modifier::NONE, Modifier::FLAT, Modifier::FLAT, Modifier::NONE, Modifier::NONE, Modifier::FLAT, Modifier::FLAT}, // Ab
	{Modifier::SHARP,Modifier::NONE, Modifier::NONE, Modifier::SHARP,Modifier::SHARP,Modifier::NONE, Modifier::NONE}, // A
	{Modifier::NONE, Modifier::NONE, Modifier::FLAT, Modifier::NONE, Modifier::NONE, Modifier::NONE, Modifier::FLAT}, // Bb
	{Modifier::SHARP,Modifier::SHARP,Modifier::NONE, Modifier::SHARP,Modifier::SHARP,Modifier::SHARP,Modifier::NONE}, // B
};

inline int scale_to_major(const Scale &s)
{
	if (s.type == Scale::Type::MAJOR)
		return s.root;
	if (s.type == Scale::Type::LOCRIAN)
		return (s.root + 1) % 12;
	if (s.type == Scale::Type::MINOR)
		return (s.root + 3) % 12;
	if (s.type == Scale::Type::MIXOLYDIAN)
		return (s.root + 5) % 12;
	if (s.type == Scale::Type::LYDIAN)
		return (s.root + 7) % 12;
	if (s.type == Scale::Type::PHRYGIAN)
		return (s.root + 8) % 12;
	if (s.type == Scale::Type::DORIAN)
		return (s.root + 10) % 12;
	return s.root;
}

Scale::Scale(int _type, int _root)
{
	type = _type;
	root = _root;
	int ms = scale_to_major(*this);
	for (int i=0; i<7; i++)
		modifiers[i] = scale_major_modifiers[ms][i];

	int offset[7] = {0, 2, 4, 5, 7, 9, 11};
	for (int i=0; i<12; i++){
		int r = (i - root + offset[type] + 24) % 12;
		// 69 = 9 = a
		_contains[i] = !((r == 10) or (r == 1) or (r == 3) or (r == 6) or (r == 8));
	}
}

string Scale::get_type_name(int type)
{
	if (type == Type::MAJOR)
		return _("Major");
	if (type == Type::DORIAN)
		return _("Dorian");
	if (type == Type::PHRYGIAN)
		return _("Phrygian");
	if (type == Type::LYDIAN)
		return _("Lydian");
	if (type == Type::MIXOLYDIAN)
		return _("Mixolydian");
	if (type == Type::MINOR)
		return _("Minor");
	if (type == Type::LOCRIAN)
		return _("Locrian");
	return "???";
}

string Scale::type_name() const
{
	return get_type_name(type);
}

bool Scale::contains(int pitch) const
{
	return _contains[pitch % 12];
}

inline int apply_modifier(int pitch, int scale_mod, int mod)
{
	int d = 0;
	if (scale_mod == Modifier::SHARP)
		d = 1;
	else if (scale_mod == Modifier::FLAT)
		d = -1;
	if (mod == Modifier::SHARP)
		d += 1;
	else if (mod == Modifier::FLAT)
		d -= 1;
	else if (mod == Modifier::NATURAL)
		d = 0;
	return pitch + d;
}

// x in major scale notation
int Scale::transform_out(int x, int mod) const
{
	const int pp[7] = {0,2,4,5,7,9,11};

	int octave = x / 7;
	int rel = x % 7;

	return apply_modifier(pitch_from_octave_and_rel(pp[rel], octave), modifiers[rel], mod);
}



