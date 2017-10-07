/*
 * PitchDetector.h
 *
 *  Created on: 13.09.2017
 *      Author: michi
 */

#ifndef SRC_AUDIO_PITCHDETECTOR_H_
#define SRC_AUDIO_PITCHDETECTOR_H_

#include "../Midi/MidiSource.h"

class AudioSource;
class AudioBuffer;

class PitchDetector : public MidiSource
{
public:
	PitchDetector(AudioSource *source);
	virtual ~PitchDetector();

	void _cdecl __init__(AudioSource *source);
	virtual void _cdecl __delete__();

	virtual int _cdecl read(MidiEventBuffer &midi);

	void process(MidiEventBuffer &midi, AudioBuffer &buf);

	AudioSource *source;

	float frequency, volume;
	bool loud_enough;
};

#endif /* SRC_AUDIO_PITCHDETECTOR_H_ */
