/*
 * BeatMidifier.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_BEATS_BEATMIDIFIER_H_
#define SRC_MODULE_BEATS_BEATMIDIFIER_H_

#include "../midi/MidiSource.h"

class Port;

class BeatMidifier : public MidiSource {
public:
	BeatMidifier();
	int read(MidiEventBuffer &midi) override;

	BeatsInPort in{this, "in"};

	float volume;
};

#endif /* SRC_MODULE_BEATS_BEATMIDIFIER_H_ */
