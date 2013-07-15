/*
 * SampleSynthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLESYNTHESIZER_H_
#define SAMPLESYNTHESIZER_H_

#include "Synthesizer.h"
#include "../../Data/Sample.h"

class SampleSynthesizer : public Synthesizer
{
public:
	SampleSynthesizer();
	virtual ~SampleSynthesizer();
	void __init__();
	virtual void __delete__();

	virtual void AddTone(BufferBox &buf, const Range &range, int pitch, float volume);
	virtual void AddClick(BufferBox &buf, int pos, int pitch, float volume);

	Array<SampleRef*> samples;
};

#endif /* SAMPLESYNTHESIZER_H_ */
