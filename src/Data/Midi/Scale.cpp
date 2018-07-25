/*
 * Scale.cpp
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#include "../../lib/hui/hui.h"
#include "Scale.h"
#include "MidiData.h"



static const NoteModifier scale_major_modifiers[12][7] = {
	{NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE}, // C
	{NoteModifier::NONE, NoteModifier::FLAT, NoteModifier::FLAT, NoteModifier::NONE, NoteModifier::FLAT, NoteModifier::FLAT, NoteModifier::FLAT}, // Db
	{NoteModifier::SHARP,NoteModifier::NONE, NoteModifier::NONE, NoteModifier::SHARP,NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE}, // D
	{NoteModifier::NONE, NoteModifier::NONE, NoteModifier::FLAT, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::FLAT, NoteModifier::FLAT}, // Eb
	{NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::NONE, NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::NONE, NoteModifier::NONE}, // E
	{NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::FLAT}, // F
	{NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::NONE}, // F#
	{NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::SHARP,NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE}, // G
	{NoteModifier::NONE, NoteModifier::FLAT, NoteModifier::FLAT, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::FLAT, NoteModifier::FLAT}, // Ab
	{NoteModifier::SHARP,NoteModifier::NONE, NoteModifier::NONE, NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::NONE, NoteModifier::NONE}, // A
	{NoteModifier::NONE, NoteModifier::NONE, NoteModifier::FLAT, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::NONE, NoteModifier::FLAT}, // Bb
	{NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::NONE, NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::SHARP,NoteModifier::NONE}, // B
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

Scale::Scale(Type _type, int _root)
{
	type = _type;
	root = _root;
	int ms = scale_to_major(*this);
	for (int i=0; i<7; i++)
		modifiers[i] = scale_major_modifiers[ms][i];

	int offset[7] = {0, 2, 4, 5, 7, 9, 11};
	for (int i=0; i<12; i++){
		int r = (i - root + offset[(int)type] + 24) % 12;
		// 69 = 9 = a
		_contains[i] = !((r == 10) or (r == 1) or (r == 3) or (r == 6) or (r == 8));
	}
}

string Scale::get_type_name(Type type)
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

inline int apply_modifier(int pitch, NoteModifier scale_mod, NoteModifier mod)
{
	int d = 0;
	if (scale_mod == NoteModifier::SHARP)
		d = 1;
	else if (scale_mod == NoteModifier::FLAT)
		d = -1;
	if (mod == NoteModifier::SHARP)
		d += 1;
	else if (mod == NoteModifier::FLAT)
		d -= 1;
	else if (mod == NoteModifier::NATURAL)
		d = 0;
	return pitch + d;
}

// x in major scale notation
int Scale::transform_out(int x, NoteModifier mod) const
{
	const int pp[7] = {0,2,4,5,7,9,11};

	int octave = x / 7;
	int rel = x % 7;

	return apply_modifier(pitch_from_octave_and_rel(pp[rel], octave), modifiers[rel], mod);
}



