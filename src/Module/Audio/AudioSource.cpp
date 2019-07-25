/*
 * AudioSource.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioSource.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"
#include "../../Stuff/PerformanceMonitor.h"


AudioSource::AudioSource() :
	Module(ModuleType::AUDIO_SOURCE, "")
{
	out = new Output(this);
	port_out.add(out);
}

void AudioSource::__init__() {
	new(this) AudioSource;
}

void AudioSource::__delete__() {
	this->AudioSource::~AudioSource();
}

AudioSource::Output::Output(AudioSource *s) : Port(SignalType::AUDIO, "out") {
	source = s;
}

int AudioSource::Output::read_audio(AudioBuffer& buf) {
	PerformanceMonitor::start_busy(source->perf_channel);
	int r = source->read(buf);
	PerformanceMonitor::end_busy(source->perf_channel);
	return r;
}

AudioSource *CreateAudioSource(Session *session, const string &name) {
	return (AudioSource*)ModuleFactory::create(session, ModuleType::AUDIO_SOURCE, name);
}
