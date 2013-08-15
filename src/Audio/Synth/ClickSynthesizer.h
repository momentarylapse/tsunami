/*
 * ClickSynthesizer.h
 *
 *  Created on: 15.08.2013
 *      Author: michi
 */

#ifndef CLICKSYNTHESIZER_H_
#define CLICKSYNTHESIZER_H_


#include "Synthesizer.h"

class ClickSynthesizer : public Synthesizer
{
public:
	ClickSynthesizer();
	virtual ~ClickSynthesizer();
	void __init__();

	virtual void RenderNote(BufferBox &buf, const Range &range, float pitch, float volume);
};

#endif /* CLICKSYNTHESIZER_H_ */
