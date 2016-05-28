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
	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual void _cdecl renderNote(BufferBox &buf, const Range &range, float pitch, float volume);

	Array<SampleRef*> samples;
};

#endif /* SAMPLESYNTHESIZER_H_ */
