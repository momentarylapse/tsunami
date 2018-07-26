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
	Module(ModuleType::AUDIO_SOURCE)
{
	out = new Output(this);
	port_out.add(PortDescription(SignalType::AUDIO, (Port**)&out, "out"));
	beat_source = nullptr;
}

AudioSource::~AudioSource()
{
	delete out;
}

void AudioSource::__init__()
{
	new(this) AudioSource;
}

void AudioSource::__delete__()
{
	this->AudioSource::~AudioSource();
}

AudioSource::Output::Output(AudioSource *s)
{
	source = s;
}

int AudioSource::Output::read(AudioBuffer& buf)
{
	return source->read(buf);
}

void AudioSource::Output::reset()
{
	source->reset();
}

int AudioSource::Output::get_pos(int delta)
{
	return source->get_pos(delta);
}

AudioSource *CreateAudioSource(Session *session, const string &name)
{
	return (AudioSource*)ModuleFactory::create(session, ModuleType::AUDIO_SOURCE, name);
}
