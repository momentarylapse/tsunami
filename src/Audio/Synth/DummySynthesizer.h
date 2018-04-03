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

	virtual void _cdecl render(AudioBuffer &buf);

	virtual void _cdecl on_config();

	virtual void _cdecl reset_state();
	struct PitchState{
		float volume;
		float phi;
		EnvelopeADSR env;
	}pitch[MAX_PITCH];

	void _set_drum(int no, float freq, float volume, float attack, float release);
};

#endif /* DUMMYSYNTHESIZER_H_ */
