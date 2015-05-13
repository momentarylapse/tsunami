/*
 * DummySynthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "DummySynthesizer.h"
#include "../../Data/AudioFile.h"
#include "../../lib/math/math.h"


void DummySynthesizer::State::reset()
{
	for (int i=0; i<128; i++){
		pitch[i].phi = 0;
		pitch[i].volume = 0;
		pitch[i].env.reset();
	}
}

DummySynthesizer::DummySynthesizer()
{
	name = "Dummy";
	state.reset();
}

DummySynthesizer::~DummySynthesizer()
{
}

void DummySynthesizer::__init__()
{
	new(this) DummySynthesizer;
}

void DummySynthesizer::onConfig()
{
	for (int i=0; i<128; i++){
		//state.pitch[i].env.set(0.01, 0.005f, 0.7f, 0.02f, sample_rate);
		state.pitch[i].env.set(0.005f, 0.01f, 0.5f, 0.02f, sample_rate);
		state.pitch[i].env.set2(0, 0.9f);
	}
}

void DummySynthesizer::render(BufferBox& buf)
{
	for (int i=0; i<buf.num; i++){

		// current events?
		foreach(MidiEvent &e, events)
			if (e.pos == i){
				int p = e.pitch;
				State::PitchState &s = state.pitch[p];
				if (e.volume == 0){
					s.env.end();
				}else{
					s.env.start(e.volume * 0.5f);
					enablePitch(p, true);
				}
			}

		for (int ip=0; ip<active_pitch.num; ip++){
			int p = active_pitch[ip];
			State::PitchState &s = state.pitch[p];

			s.volume = s.env.get();

			if (s.volume == 0){
				enablePitch(p, false);
				continue;
			}

			float d = sin(s.phi) * s.volume;
			buf.r[i] += d;
			buf.l[i] += d;

			s.phi += delta_phi[p];
			if (s.phi > 8*pi)
				s.phi = loopf(s.phi, 0, 2*pi);
		}
	}
}
