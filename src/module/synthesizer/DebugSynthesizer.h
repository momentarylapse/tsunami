/*
 * DebugSynthesizer.h
 *
 *  Created on: 22 Jan 2022
 *      Author: michi
 */

#ifndef SRC_MODULE_SYNTH_DEBUGSYNTHESIZER_H_
#define SRC_MODULE_SYNTH_DEBUGSYNTHESIZER_H_

#include "Synthesizer.h"
#include "EnvelopeADSR.h"


class DebugSynthesizer : public Synthesizer {
public:
	DebugSynthesizer();

	PitchRenderer *create_pitch_renderer(int pitch) override;
	int ttl = 0;
};

#endif /* SRC_MODULE_SYNTH_DEBUGSYNTHESIZER_H_ */
