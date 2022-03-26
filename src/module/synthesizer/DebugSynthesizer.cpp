/*
 * DebugSynthesizer.cpp
 *
 *  Created on: 22 Jan 2022
 *      Author: michi
 */

#include "DebugSynthesizer.h"
#include "../../data/audio/AudioBuffer.h"


class DebugPitchRenderer : public PitchRenderer {
public:
	DebugPitchRenderer(Synthesizer *synth, int pitch) : PitchRenderer(synth, pitch) {
	}
	bool _cdecl render(AudioBuffer &buf) override {
		for (int i=0; i<buf.length; i++) {
			buf.c[0][i] += (i%2==0) ? 1 : -1;

			if (ttl >= 0) {
				if (ttl == 0)
					return false;
				ttl --;
			}
		}
		return true;
	}
	void on_end() override {
		ttl = static_cast<DebugSynthesizer*>(synth)->ttl;
	}
	void on_start(float volume) override {
		ttl = -1;
	}
	int ttl = -1;
};


DebugSynthesizer::DebugSynthesizer() {
	module_class = "Debug";
	auto_generate_stereo = true;
}

PitchRenderer *DebugSynthesizer::create_pitch_renderer(int pitch) {
	return new DebugPitchRenderer(this, pitch);
}
