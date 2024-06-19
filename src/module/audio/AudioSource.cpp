/*
 * AudioSource.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioSource.h"
#include "../ModuleFactory.h"
#include "../../data/base.h"

namespace tsunami {

AudioSource::AudioSource() :
	Module(ModuleCategory::AUDIO_SOURCE, "")
{
}

void AudioSource::__init__() {
	new(this) AudioSource;
}

void AudioSource::__delete__() {
	this->AudioSource::~AudioSource();
}

int AudioSource::read_audio(int port, AudioBuffer& buf) {
	perf_start();
	int r = read(buf);
	perf_end();
	return r;
}

AudioSource *CreateAudioSource(Session *session, const string &name) {
	return (AudioSource*)ModuleFactory::create(session, ModuleCategory::AUDIO_SOURCE, name);
}

}
