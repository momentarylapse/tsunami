/*
 * ClickSynthesizer.cpp
 *
 *  Created on: 15.08.2013
 *      Author: michi
 */

#include "ClickSynthesizer.h"
#include "../../Data/AudioFile.h"
#include "../../lib/math/math.h"

ClickSynthesizer::ClickSynthesizer()
{
	name = "ClickSynthesizer";
}

ClickSynthesizer::~ClickSynthesizer()
{
}

void ClickSynthesizer::__init__()
{
	new(this) ClickSynthesizer;
}

void ClickSynthesizer::RenderNote(BufferBox& buf, const Range& range, float pitch, float volume)
{
	float freq = pitch_to_freq(pitch);
	float f_w = 1.0f / sample_rate * freq * 2.0f * pi;
	int sm_d = 0.02f * sample_rate;
	keep_notes = sm_d * 8;

	int i0 = max(range.offset, 0);
	int i1 = min(range.offset + sm_d * 8, buf.num);

	for (int i=i0; i<i1; i++){
		float tt = (i - range.offset) * f_w;
		float d = sin(tt) * volume;
		float fi = (float)(i - range.end()) / (float)sm_d;
		d *= exp(-fi);
		buf.r[i] += d;
		buf.l[i] += d;
	}
}
