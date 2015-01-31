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
		pitch[i].phase = 0;
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
				if (e.volume == 0){
					state.pitch[p].fading = true;
					state.pitch[p].lin_range = -1;
				}else{
					state.pitch[p].fading = false;
					state.pitch[p].lin_range = 100;
					state.pitch[p].lin_step = (e.volume - state.pitch[p].volume) / (float)state.pitch[p].lin_range;
				}
			}

		for (int p=0; p<128; p++){

			if (state.pitch[p].fading){
				state.pitch[p].volume *= exp(-sm_d);
				if (state.pitch[p].volume <= 0.001f){
					state.pitch[p].volume = 0;
					state.pitch[p].fading = false;
				}
			}else if (state.pitch[p].lin_range >= 0){
				state.pitch[p].volume += state.pitch[p].lin_step;
				state.pitch[p].lin_range --;
			}

			if (state.pitch[p].volume == 0)
				continue;

			float freq = pitch_to_freq(p);
			float dphase = freq * 2.0f * pi / sample_rate;

			float d = sin(state.pitch[p].phase) * state.pitch[p].volume;
			buf.r[i] += d;
			buf.l[i] += d;

			state.pitch[p].phase += dphase;
			if (state.pitch[p].phase > 2*pi)
				state.pitch[p].phase -= 2*pi;
		}
	}
}
