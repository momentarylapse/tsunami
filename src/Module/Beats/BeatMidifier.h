/*
 * BeatMidifier.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_BEATS_BEATMIDIFIER_H_
#define SRC_MODULE_BEATS_BEATMIDIFIER_H_

#include "../Midi/MidiSource.h"

class BeatMidifier : public MidiSource
{
public:
	BeatMidifier();
	virtual int _cdecl read(MidiEventBuffer &midi);
	virtual void _cdecl reset();
};

#endif /* SRC_MODULE_BEATS_BEATMIDIFIER_H_ */
