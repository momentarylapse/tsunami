/*
 * DummySynthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "DummySynthesizer.h"
#include "../../Data/AudioFile.h"
#include "../../lib/math/math.h"

DummySynthesizer::DummySynthesizer()
{
	name = "DummySynthesizer";
}

DummySynthesizer::~DummySynthesizer()
{
}

void DummySynthesizer::__init__()
{
	new(this) DummySynthesizer;
}

void DummySynthesizer::AddTone(BufferBox& buf, const Range& range, float pitch, float volume)
{
	float freq = pitch_to_freq(pitch);
	float f_w = 1.0f / sample_rate * freq * 2.0f * pi;
	int sm_d = 0.02f * sample_rate;
	keep_notes = sm_d * 8;

	int i0 = max(range.offset, 0);
	int i1 = min(range.end() + sm_d * 8, buf.num);

	for (int i=i0; i<i1; i++){
		float tt = (i - range.offset) * f_w;
		float d = sin(tt) * volume;
		if (i > range.end()){
			float fi = (float)(i - range.end()) / (float)sm_d;
			d *= exp(-fi);//1 - fi;
		}else if (i < range.offset + 1000){
			d *= (i - range.offset) * 0.001;
		}
		buf.r[i] += d;
		buf.l[i] += d;
	}
}

/*void DummySynthesizer::AddClick(BufferBox &buf, int pos, float pitch, float volume)
{
	float freq = pitch_to_freq(pitch);
	int sm_d = 0.06f * sample_rate;
	float w_f = 1.0 / sample_rate * freq * 2.0 * 3.14159265358979f;

	for (int i=0; i<sm_d; i++){
		float fi = (float)i / (float)sm_d;
		float envelope = exp(-fi*3);//1 - fi;
		float tt = i * w_f;
		int j = i + pos;// - offset;
		if ((j >= 0) && (j < buf.num)){
			float d = sin(tt) * volume * envelope;
			buf.r[j] += d;
			buf.l[j] += d;
		}
	}
}*/
