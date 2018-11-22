/*
 * Scale.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_SCALE_H_
#define SRC_DATA_MIDI_SCALE_H_

enum class NoteModifier;

class Scale
{
public:

	enum class Type{
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

	static string get_type_name(Type type);
	string type_name() const;
};


int uniclef_get_rel(int upos);
int uniclef_get_octave(int upos);
int uniclef_to_pitch(int upos);
int uniclef_to_pitch(int upos, NoteModifier mod);





#endif /* SRC_DATA_MIDI_SCALE_H_ */
