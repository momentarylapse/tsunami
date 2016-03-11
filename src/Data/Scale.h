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
		NUM_TYPES,
		TYPE_CUSTOM = -1
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





#endif /* SRC_DATA_SCALE_H_ */
