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
#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"
#include "../../Stuff/PerformanceMonitor.h"


AudioEffect::Output::Output(AudioEffect *_fx) : Port(SignalType::AUDIO, "out") {
	fx = _fx;
}

int AudioEffect::Output::read_audio(AudioBuffer &buf) {
	return fx->read(buf);
}

AudioEffect::AudioEffect() :
	Module(ModuleType::AUDIO_EFFECT, "")
{
	source = nullptr;
	out = new Output(this);
	port_out.add(out);
	port_in.add(InPortDescription(SignalType::AUDIO, &source, "in"));
	sample_rate = DEFAULT_SAMPLE_RATE;
}

void AudioEffect::__init__() {
	new(this) AudioEffect;
}

void AudioEffect::__delete__() {
	this->AudioEffect::~AudioEffect();
}

int AudioEffect::read(AudioBuffer &buf) {
	if (!source)
		return buf.length;
	sample_rate = session->sample_rate();
	int samples = source->read_audio(buf);
	PerformanceMonitor::start_busy(perf_channel);
	if (samples > 0)
		process(buf);
	PerformanceMonitor::end_busy(perf_channel);
	return samples;
}


AudioEffect *CreateAudioEffect(Session *session, const string &name) {
	return (AudioEffect*)ModuleFactory::create(session, ModuleType::AUDIO_EFFECT, name);
}
