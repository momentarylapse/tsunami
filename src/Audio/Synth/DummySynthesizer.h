/*
 * DummySynthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef DUMMYSYNTHESIZER_H_
#define DUMMYSYNTHESIZER_H_

#include "Synthesizer.h"

class DummySynthesizer : public Synthesizer
{
public:
	DummySynthesizer();
	virtual ~DummySynthesizer();

	virtual void AddToneFreq(BufferBox &buf, const Range &range, float freq, float volume);
	virtual void AddClick(BufferBox &buf, int pos, int pitch, float volume);
};

#endif /* DUMMYSYNTHESIZER_H_ */
