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
	int _cdecl read(MidiEventBuffer &midi) override;
	void _cdecl reset() override;

	float volume;
};

BeatMidifier *_cdecl CreateBeatMidifier(Session *session);

#endif /* SRC_MODULE_BEATS_BEATMIDIFIER_H_ */
