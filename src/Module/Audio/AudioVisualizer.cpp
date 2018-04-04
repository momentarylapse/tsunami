/*
 * AudioVisualizer.cpp
 *
 *  Created on: 04.04.2018
 *      Author: michi
 */

#include "AudioVisualizer.h"
#include "../../Session.h"
#include "../../Plugins/Plugin.h"
#include "../../Plugins/PluginManager.h"
#include "PeakMeter.h"

AudioVisualizer::Output::Output(AudioVisualizer *v)
{
	visualizer = v;
}

int AudioVisualizer::Output::read(AudioBuffer& buf)
{
	if (!visualizer->source)
		return 0;
	int r = visualizer->source->read(buf);
	if (r <= 0)
		return r;

	visualizer->process(buf);
	return r;
}

int AudioVisualizer::Output::get_pos(int delta)
{
	if (!visualizer->source)
		return 0;
	return visualizer->source->get_pos(delta);
}

void AudioVisualizer::Output::reset()
{
	if (visualizer->source)
		visualizer->source->reset();
	visualizer->reset();
}

AudioVisualizer::AudioVisualizer() :
	Module(Session::GLOBAL, Type::AUDIO_VISUALIZER)
{
	out = new Output(this);
	port_out.add(PortDescription(SignalType::AUDIO, (Port**)&out, "out"));
	port_in.add(PortDescription(SignalType::AUDIO, (Port**)&source, "in"));
	source = NULL;
}

AudioVisualizer::~AudioVisualizer()
{
	delete out;
}

void AudioVisualizer::__init__()
{
	new(this) AudioVisualizer;
}

void AudioVisualizer::__delete__()
{
	this->AudioVisualizer::~AudioVisualizer();
}

void AudioVisualizer::set_source(AudioPort *s)
{
	source = s;
}

// TODO: move to PluginManager?
AudioVisualizer *CreateAudioVisualizer(Session *session, const string &name)
{
	AudioVisualizer *s = NULL;

	if (name == "PeakMeter"){
		s = new PeakMeter(session);
	}else{
		Plugin *p = session->plugin_manager->GetPlugin(session, Plugin::Type::AUDIO_VISUALIZER, name);
		if (p->usable)
			s = (AudioVisualizer*)p->create_instance(session, "AudioVisualizer");
		s->plugin = p;
		s->usable = p->usable;
	}

	// dummy?
	if (!s)
		s = new AudioVisualizer;

	s->name = name;
	s->session = session;
	s->reset_config();
	return s;
}


