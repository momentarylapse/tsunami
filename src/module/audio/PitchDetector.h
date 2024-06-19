/*
 * PitchDetector.h
 *
 *  Created on: 13.09.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_AUDIO_PITCHDETECTOR_H_
#define SRC_MODULE_AUDIO_PITCHDETECTOR_H_

#include "../midi/MidiSource.h"

namespace tsunami {

class Port;
class AudioBuffer;

class PitchDetector : public MidiSource {
public:
	PitchDetector();

	void _cdecl __init__();

	int _cdecl read(MidiEventBuffer &midi) override;

	virtual void process(MidiEventBuffer &midi, AudioBuffer &buf) {};

	AudioInPort in{this, "in"};

	float frequency, volume;
	int pitch;
	bool loud_enough;
};

class DummyPitchDetector : public PitchDetector {
public:
	DummyPitchDetector();

	void _cdecl __init__();
	void process(MidiEventBuffer &midi, AudioBuffer &buf) override;
};

}

#endif /* SRC_MODULE_AUDIO_PITCHDETECTOR_H_ */
