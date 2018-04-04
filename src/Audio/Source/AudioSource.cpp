/*
 * AudioSource.cpp
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#include "AudioSource.h"
#include "../../Session.h"
#include "../../Plugins/Plugin.h"
#include "../../Plugins/PluginManager.h"
#include "SongRenderer.h"

AudioSource::AudioSource() :
	Module(Session::GLOBAL, Type::AUDIO_SOURCE)
{
	out = new Output(this);
	port_out.add(PortDescription(SignalType::AUDIO, (Port**)&out, "out"));
	beat_source = NULL;
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

// TODO: move to PluginManager?
AudioSource *CreateAudioSource(Session *session, const string &name)
{
	AudioSource *s = NULL;

	if (name == "SongRenderer"){
		s = new SongRenderer(session->song);
	}else{
		Plugin *p = session->plugin_manager->GetPlugin(session, Plugin::Type::AUDIO_SOURCE, name);
		if (p->usable)
			s = (AudioSource*)p->create_instance(session, "AudioSource");
		s->plugin = p;
		s->usable = p->usable;
	}

	// dummy?
	if (!s)
		s = new AudioSource;

	s->name = name;
	s->session = session;
	s->reset_config();
	return s;
}
