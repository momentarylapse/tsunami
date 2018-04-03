/*
 * AudioEffect.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "AudioEffect.h"

#include "Plugin.h"
#include "../Session.h"
#include "../lib/math/math.h"
#include "PluginManager.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"


AudioEffect::Output::Output(AudioEffect *_fx)
{
	fx = _fx;
}

int AudioEffect::Output::read(AudioBuffer &buf)
{
	if (!fx->source)
		return buf.length;
	fx->sample_rate = fx->session->sample_rate();
	int samples = fx->source->read(buf);
	if (samples > 0)
		fx->process(buf);
	return samples;
}

void AudioEffect::Output::reset()
{
	fx->reset_state();
	if (fx->source)
		fx->source->reset();
}

int AudioEffect::Output::get_pos(int delta)
{
	if (!fx->source)
		return -1;
	return fx->source->get_pos(delta);
}

AudioEffect::AudioEffect() :
	Configurable(Session::GLOBAL, Type::AUDIO_EFFECT)
{
	source = NULL;
	out = new Output(this);
	sample_rate = DEFAULT_SAMPLE_RATE;
}

AudioEffect::~AudioEffect()
{
	delete out;
}

void AudioEffect::__init__()
{
	new(this) AudioEffect;
}

void AudioEffect::__delete__()
{
	this->AudioEffect::~AudioEffect();
}

void AudioEffect::set_source(AudioPort *_source)
{
	source = _source;
}

void AudioEffect::do_process_track(Track *t, int layer, const Range &r)
{
	sample_rate = t->song->sample_rate;
	AudioBuffer buf = t->getBuffers(layer, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, layer, r);
	process(buf);
	session->song->execute(a);
}


// TODO: move to PluginManager?
AudioEffect *CreateAudioEffect(Session *session, const string &name)
{
	Plugin *p = session->plugin_manager->GetPlugin(session, Plugin::Type::AUDIO_EFFECT, name);
	AudioEffect *fx = NULL;
	if (p->usable)
		fx = (AudioEffect*)p->create_instance(session, "AudioEffect");

	// dummy?
	if (!fx)
		fx = new AudioEffect;

	fx->name = name;
	fx->plugin = p;
	fx->usable = p->usable;
	fx->song = session->song;
	fx->session = session;
	fx->reset_config();
	return fx;
}
