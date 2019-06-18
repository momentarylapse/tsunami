/*
 * SampleSynthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SRC_MODULE_SYNTH_SAMPLESYNTHESIZER_H_
#define SRC_MODULE_SYNTH_SAMPLESYNTHESIZER_H_

#include "Synthesizer.h"
#include "../../Data/Sample.h"

class SampleSynthesizer : public Synthesizer {
public:
	SampleSynthesizer();
	virtual ~SampleSynthesizer();
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual void _cdecl renderNote(AudioBuffer &buf, const Range &range, float pitch, float volume);

	Array<SampleRef*> samples;
};

#endif /* SRC_MODULE_SYNTH_SAMPLESYNTHESIZER_H_ */
