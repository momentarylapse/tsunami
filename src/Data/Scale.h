/*
 * Scale.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_SCALE_H_
#define SRC_DATA_SCALE_H_

class Scale
{
public:

	enum
	{
		TYPE_MAJOR,
		TYPE_DORIAN,
		TYPE_PHRYGIAN,
		TYPE_LYDIAN,
		TYPE_MIXOLYDIAN,
		TYPE_MINOR,
		TYPE_LOCRIAN,
		NUM_TYPES
	};
	int type, root;

	Scale(int type, int root);
	bool contains(int pitch) const;
	const int* get_modifiers_clef() const;
	//int* get_modifiers_pitch();
	int transform_out(int x, int modifier) const;

	static string get_type_name(int type);
	string type_name() const;
};





#endif /* SRC_DATA_SCALE_H_ */
