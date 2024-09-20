/*
 * AudioEffect.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "AudioEffect.h"
#include "../ModuleFactory.h"
#include "../../Session.h"
#include "../../action/track/buffer/ActionTrackEditBuffer.h"
#include "../../data/base.h"
#include "../../data/TrackLayer.h"

namespace tsunami {

int AudioEffect::read_audio(int port, AudioBuffer &buf) {
	return read(buf);
}

AudioEffect::AudioEffect() :
	Module(ModuleCategory::AudioEffect, "")
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	apply_to_whole_buffer = false;
	wetness = 1.0f;
}

void AudioEffect::__init__() {
	new(this) AudioEffect;
}

void AudioEffect::__delete__() {
	this->AudioEffect::~AudioEffect();
}

// assumes buf_out already correctly sized etc!
void combine_buffers(AudioBuffer& buf_out, AudioBuffer& a, float fa, const AudioBuffer& b, float fb) {
	for (int c=0; c<a.channels; c++)
		for (int i=0; i<a.length; i++)
			buf_out.c[c][i] = a.c[c][i] * fa + b.c[c][i] * fb;
}

void AudioEffect::apply_with_wetness(AudioBuffer &buf) {
	if (wetness <= 0.0f or !enabled)
		return;
	if (wetness > 0.99f) {
		process(buf);
		return;
	}

	AudioBuffer buf_wet = buf;
	process(buf_wet);

	//combine_buffers(buf, buf, (1 - wetness), buf_wet, wetness);
	buf.scale(1 - wetness);
	buf.add(buf_wet, 0, wetness);
}

int AudioEffect::read(AudioBuffer &buf) {
	sample_rate = session->sample_rate();
	int samples = in.source->read_audio(buf);
	
	perf_start();
	if (samples > 0 and enabled)
		apply_with_wetness(buf);
	perf_end();
	return samples;
}


AudioEffect *CreateAudioEffect(Session *session, const string &name) {
	return (AudioEffect*)ModuleFactory::create(session, ModuleCategory::AudioEffect, name);
}

}
