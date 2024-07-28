/*
 * Scale.cpp
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#include "../../lib/hui/hui.h"
#include "Scale.h"
#include "MidiData.h"


namespace tsunami {

const Scale Scale::C_MAJOR = Scale(Scale::Type::MAJOR, 0);

static const NoteModifier scale_major_modifiers[12][7] = {
	{NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::None}, // C
	{NoteModifier::None, NoteModifier::Flat, NoteModifier::Flat, NoteModifier::None, NoteModifier::Flat, NoteModifier::Flat, NoteModifier::Flat}, // Db
	{NoteModifier::Sharp,NoteModifier::None, NoteModifier::None, NoteModifier::Sharp,NoteModifier::None, NoteModifier::None, NoteModifier::None}, // D
	{NoteModifier::None, NoteModifier::None, NoteModifier::Flat, NoteModifier::None, NoteModifier::None, NoteModifier::Flat, NoteModifier::Flat}, // Eb
	{NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::None, NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::None, NoteModifier::None}, // E
	{NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::Flat}, // F
	{NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::None}, // F#
	{NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::Sharp,NoteModifier::None, NoteModifier::None, NoteModifier::None}, // G
	{NoteModifier::None, NoteModifier::Flat, NoteModifier::Flat, NoteModifier::None, NoteModifier::None, NoteModifier::Flat, NoteModifier::Flat}, // Ab
	{NoteModifier::Sharp,NoteModifier::None, NoteModifier::None, NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::None, NoteModifier::None}, // A
	{NoteModifier::None, NoteModifier::None, NoteModifier::Flat, NoteModifier::None, NoteModifier::None, NoteModifier::None, NoteModifier::Flat}, // Bb
	{NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::None, NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::Sharp,NoteModifier::None}, // B
};

inline int scale_to_major(const Scale &s) {
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

Scale::Scale(Type _type, int _root) {
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

// can be parsed
string Scale::get_type_name_canonical(Type type) {
	if (type == Type::MAJOR)
		return "major";
	if (type == Type::DORIAN)
		return "dorian";
	if (type == Type::PHRYGIAN)
		return "phrygian";
	if (type == Type::LYDIAN)
		return "lydian";
	if (type == Type::MIXOLYDIAN)
		return "mixolydian";
	if (type == Type::MINOR)
		return "minor";
	if (type == Type::LOCRIAN)
		return "locrian";
	return "???";
}

string Scale::get_type_name(Type type) {
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

string Scale::nice_name() const {
	return rel_pitch_name(root) + " " + type_name();
}

string Scale::type_name() const {
	return get_type_name(type);
}

string Scale::encode() const {
	return rel_pitch_name_canonical(root) + "-" + get_type_name_canonical(type);
}

Scale Scale::parse(const string &text) {
	auto ss = text.explode("-");
	if (ss.num != 2)
		return Scale::C_MAJOR;
	int root = max(parse_rel_pitch(ss[0]), 0);
	Type type = Type::MAJOR;
	for (int i=0; i<(int)Scale::Type::NUM_TYPES; i++)
		if (ss[1] == Scale::get_type_name_canonical((Scale::Type)i))
			type = (Scale::Type)i;
	return Scale(type, root);
}

bool Scale::contains(int pitch) const {
	return _contains[pitch % 12];
}

int uniclef_get_rel(int upos) {
	return upos % 7;
}

int uniclef_get_octave(int upos) {
	return upos / 7;
}

// "major scale notation"
int uniclef_to_pitch(int upos) {
	const int pp[7] = {0,2,4,5,7,9,11};

	int octave = uniclef_get_octave(upos);
	int rel = uniclef_get_rel(upos);
	return pitch_from_octave_and_rel(pp[rel], octave);
}

int uniclef_to_pitch(int upos, NoteModifier mod) {
	return uniclef_to_pitch(upos) + modifier_shift(mod);
}

// upos in major scale notation
int Scale::transform_out(int upos, NoteModifier mod) const {
	int pitch = uniclef_to_pitch(upos);
	return modifier_apply(pitch, mod, get_modifier(upos));
}

NoteModifier Scale::get_modifier(int upos) const {
	int rel = uniclef_get_rel(upos);
	return modifiers[rel];
}

}



