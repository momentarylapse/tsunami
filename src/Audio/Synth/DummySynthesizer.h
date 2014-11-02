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
	void __init__();

	virtual void renderNote(BufferBox &buf, const Range &range, float pitch, float volume);
};

#endif /* DUMMYSYNTHESIZER_H_ */
