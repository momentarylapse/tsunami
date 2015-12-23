/*
 * DummySynthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef DUMMYSYNTHESIZER_H_
#define DUMMYSYNTHESIZER_H_

#include "Synthesizer.h"
#include "EnvelopeADSR.h"

class DummySynthesizer : public Synthesizer
{
public:
	DummySynthesizer();
	virtual ~DummySynthesizer();
	void __init__();

	virtual void _cdecl render(BufferBox &buf);

	virtual void _cdecl onConfig();

	class State : public PluginData
	{
	public:
		virtual void reset();
		struct PitchState{
			float volume;
			float phi;
			EnvelopeADSR env;
		}pitch[MAX_PITCH];
	};

	State state;
};

#endif /* DUMMYSYNTHESIZER_H_ */
