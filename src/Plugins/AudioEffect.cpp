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
#include "../Stuff/Log.h"
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
	int samples = fx->source->read(buf);
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

int AudioEffect::Output::sample_rate()
{
	return fx->sample_rate;
}

AudioEffect::AudioEffect() :
	Configurable(Session::GLOBAL, Type::AUDIO_EFFECT)
{
	usable = true;
	plugin = NULL;
	enabled = true;
	sample_rate = DEFAULT_SAMPLE_RATE;
	source = NULL;
	out = new Output(this);
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
	if (source)
		sample_rate = source->sample_rate();
}

/*void Effect::prepare()
{
	resetState();
}*/

string AudioEffect::getError()
{
	if (plugin)
		return plugin->getError();
	return format(_("Can't load effect: \"%s\""), name.c_str());
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
	Plugin *p = session->plugin_manager->GetPlugin(session, Plugin::Type::EFFECT, name);
	AudioEffect *fx = NULL;
	if (p->usable)
		fx = (AudioEffect*)p->createInstance(session, "AudioEffect");

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
