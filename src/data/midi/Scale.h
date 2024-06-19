/*
 * Scale.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_SCALE_H_
#define SRC_DATA_MIDI_SCALE_H_


namespace tsunami {

enum class NoteModifier;

class Scale {
public:

	enum class Type {
		MAJOR,
		DORIAN,
		PHRYGIAN,
		LYDIAN,
		MIXOLYDIAN,
		MINOR,
		LOCRIAN,
		NUM_TYPES,
		CUSTOM = -1
	};
	Type type;
	int root;
	NoteModifier modifiers[7];
	bool _contains[12];

	Scale(Type type, int root);
	bool contains(int pitch) const;
	int transform_out(int upos, NoteModifier modifier) const;

	NoteModifier get_modifier(int upos) const;

	string nice_name() const;
	string type_name() const;
	string encode() const;

	static string get_type_name_canonical(Type type);
	static string get_type_name(Type type);
	static Scale parse(const string &text);
	static const Scale C_MAJOR;
};


int uniclef_get_rel(int upos);
int uniclef_get_octave(int upos);
int uniclef_to_pitch(int upos);
int uniclef_to_pitch(int upos, NoteModifier mod);



}

#endif /* SRC_DATA_MIDI_SCALE_H_ */
