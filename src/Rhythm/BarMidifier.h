/*
 * BarMidifier.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_RHYTHM_BARMIDIFIER_H_
#define SRC_RHYTHM_BARMIDIFIER_H_

#include "../Midi/MidiSource.h"

class BeatMidifier : public MidiSource
{
public:
	BeatMidifier();
	virtual int _cdecl read(MidiEventBuffer &midi);
	virtual void _cdecl reset();
};

#endif /* SRC_RHYTHM_BARMIDIFIER_H_ */
