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
		pitch[i].fading = false;
		pitch[i].lin_range = -1;
	}
}

DummySynthesizer::DummySynthesizer()
{
	name = "Dummy";
}

DummySynthesizer::~DummySynthesizer()
{
}

void DummySynthesizer::__init__()
{
	new(this) DummySynthesizer;
}

void DummySynthesizer::render(BufferBox& buf)
{
	float sm_d = 1.0f/(0.02f * sample_rate);

	for (int i=0; i<buf.num; i++){

		// current events?
		foreach(MidiEvent &e, events)
			if (e.pos == i){
				int p = e.pitch;
				State::PitchState &s = state.pitch[p];
				if (e.volume == 0){
					s.fading = true;
					s.lin_range = -1;
				}else{
					s.fading = false;
					s.lin_range = 100;
					s.lin_step = (e.volume - s.volume) / (float)s.lin_range;
					enablePitch(p, true);
				}
			}

		for (int ip=0; ip<active_pitch.num; ip++){
			int p = active_pitch[ip];
			State::PitchState &s = state.pitch[p];

			if (s.fading){
				s.volume *= exp(-sm_d);
				if (s.volume <= 0.001f){
					s.volume = 0;
					s.fading = false;
				}
			}else if (s.lin_range >= 0){
				s.volume += s.lin_step;
				s.lin_range --;
			}

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
