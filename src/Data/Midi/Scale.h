/*
 * Scale.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDI_SCALE_H_
#define SRC_DATA_MIDI_SCALE_H_

class Scale
{
public:

	enum Type{
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
	int type, root;
	int modifiers[7];
	bool _contains[12];

	Scale(int type, int root);
	bool contains(int pitch) const;
	int transform_out(int x, int modifier) const;

	static string get_type_name(int type);
	string type_name() const;
};





#endif /* SRC_DATA_MIDI_SCALE_H_ */
