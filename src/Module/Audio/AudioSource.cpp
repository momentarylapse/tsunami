/*
 * AudioSource.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioSource.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"

AudioSource::AudioSource() :
	Module(ModuleCategory::AUDIO_SOURCE, "")
{
	port_out.add(new Output(this));
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
	source->perf_start();
	int r = source->read(buf);
	source->perf_end();
	return r;
}

AudioSource *CreateAudioSource(Session *session, const string &name) {
	return (AudioSource*)ModuleFactory::create(session, ModuleCategory::AUDIO_SOURCE, name);
}
