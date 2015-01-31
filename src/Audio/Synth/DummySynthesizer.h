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

	virtual void render(BufferBox &buf);

	class State : public PluginData
	{
	public:
		virtual void reset();
		struct PitchState{
			float volume;
			float phase;
			bool fading;
			float lin_step;
			int lin_range;
		}pitch[128];
	};

	State state;
};

#endif /* DUMMYSYNTHESIZER_H_ */
