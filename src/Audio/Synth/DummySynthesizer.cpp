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
	int sm_d = 0.02f * sample_rate;

	for (int p=0; p<128; p++){
		float freq = pitch_to_freq(p);
		float dphase = 1.0f / sample_rate * freq * 2.0f * pi;
		//float f_w = 1.0f / sample_rate * freq * 2.0f * pi;
		//keep_notes = sm_d * 8;

		//int i0 = max(range.offset, 0);
		//int i1 = min(range.end() + sm_d * 8, buf.num);

		for (int i=0; i<buf.num; i++){
			if (state.pitch[p].fading){
				state.pitch[p].volume *= exp(-sm_d);
				if (state.pitch[p].volume <= 0.001f){
					state.pitch[p].volume = 0;
					state.pitch[p].fading = false;
				}
			}
			msg_write(events[p].num);
			foreach(MidiEvent &e, events[p])
				if (e.pos == i){
					msg_write("ppp");
					if (e.volume == 0){
						state.pitch[p].fading = true;
					}else{
						state.pitch[p].fading = false;
						state.pitch[p].volume = e.volume;
					}
				}

			float d = sin(state.pitch[p].phase) * state.pitch[p].volume;
			/*if (i < range.offset + 1000){
				d *= (i - range.offset) * 0.001;
			}*/
			buf.r[i] += d;
			buf.l[i] += d;

			state.pitch[p].phase += dphase;
			if (state.pitch[p].phase > 2*pi)
				state.pitch[p].phase -= 2*pi;
		}

	/*for (int i=i0; i<i1; i++){
		float tt = (i - range.offset) * f_w;
		float d = sin(tt) * volume;
		if (i > range.end()){
			float fi = (float)(i - range.end()) / (float)sm_d;
			d *= exp(-fi);//1 - fi;
		}else if (i < range.offset + 1000){
			d *= (i - range.offset) * 0.001;
		}
		buf.r[i] += d;
		buf.l[i] += d;*/
	}
}
