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
	name = "Dummy";
}

DummySynthesizer::~DummySynthesizer()
{
}

void DummySynthesizer::__init__()
{
	new(this) DummySynthesizer;
}

void DummySynthesizer::renderNote(BufferBox& buf, const Range& range, float pitch, float volume)
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
