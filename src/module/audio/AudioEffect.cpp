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
}

void AudioEffect::__init__() {
	new(this) AudioEffect;
}

void AudioEffect::__delete__() {
	this->AudioEffect::~AudioEffect();
}

int AudioEffect::read(AudioBuffer &buf) {
	sample_rate = session->sample_rate();
	int samples = source->read_audio(buf);
	
	perf_start();
	if (samples > 0)
		process(buf);
	perf_end();
	return samples;
}


AudioEffect *CreateAudioEffect(Session *session, const string &name) {
	return (AudioEffect*)ModuleFactory::create(session, ModuleCategory::AUDIO_EFFECT, name);
}
