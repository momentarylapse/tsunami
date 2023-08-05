/*
 * AudioEffect.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "AudioEffect.h"
#include "../ModuleFactory.h"
#include "../../Session.h"
#include "../../lib/math/math.h"
#include "../../action/track/buffer/ActionTrackEditBuffer.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/TrackLayer.h"


AudioEffect::Output::Output(AudioEffect *_fx) : Port(SignalType::AUDIO, "out") {
	fx = _fx;
}

int AudioEffect::Output::read_audio(AudioBuffer &buf) {
	return fx->read(buf);
}

AudioEffect::AudioEffect() :
	Module(ModuleCategory::AUDIO_EFFECT, "")
{
	source = nullptr;
	port_out.add(new Output(this));
	port_in.add({SignalType::AUDIO, &source, "in"});
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
void combine_buffers(AudioBuffer &buf_out, const AudioBuffer& a, float fa, const AudioBuffer& b, float fb) {
	for (int c=0; c<a.channels; c++)
		for (int i=0; i<a.length; i++)
			buf_out.c[c][i] = a.c[c][i] * fa + b.c[c][i] * fb;
}

void AudioEffect::apply_with_wetness(AudioBuffer &buf) {
	if (wetness <= 0.0f or !enabled)
		return;
	if (wetness == 1.0f)
		process(buf);

	AudioBuffer buf_wet = buf;
	process(buf_wet);

	combine_buffers(buf, buf, (1 - wetness), buf_wet, wetness);
}

int AudioEffect::read(AudioBuffer &buf) {
	sample_rate = session->sample_rate();
	int samples = source->read_audio(buf);
	
	perf_start();
	if (samples > 0 and enabled)
		apply_with_wetness(buf);
	perf_end();
	return samples;
}


AudioEffect *CreateAudioEffect(Session *session, const string &name) {
	return (AudioEffect*)ModuleFactory::create(session, ModuleCategory::AUDIO_EFFECT, name);
}
