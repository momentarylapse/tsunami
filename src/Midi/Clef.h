/*
 * Clef.h
 *
 *  Created on: 11.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_CLEF_H_
#define SRC_DATA_CLEF_H_

#include "../lib/base/base.h"

class Scale;

class Clef
{
public:
	Clef(int type, const string &symbol, int offset);
	enum{
		TYPE_TREBLE,
		TYPE_TREBLE_8,
		TYPE_BASS,
		TYPE_BASS_8,
		TYPE_DRUMS,
		NUM_CLEF_TYPES
	};

	static const Clef TREBLE;
	static const Clef TREBLE_8;
	static const Clef BASS;
	static const Clef BASS_8;
	static const Clef DRUMS;

	int type;
	int offset;
	string symbol;

	int pitch_to_position(int pitch, const Scale &s, int &modifier) const;
	int position_to_pitch(int position, const Scale &s, int modifier) const;
};

#endif /* SRC_DATA_CLEF_H_ */
