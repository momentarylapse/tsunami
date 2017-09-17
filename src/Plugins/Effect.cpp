/*
 * Effect.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Effect.h"

#include "Plugin.h"
#include "../Tsunami.h"
#include "../lib/math/math.h"
#include "../Stuff/Log.h"
#include "PluginManager.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"


Effect::Output::Output(Effect *_fx)
{
	fx = _fx;
	source = NULL;
}

int Effect::Output::read(AudioBuffer &buf)
{
	if (!source)
		return 0;
	int samples = source->read(buf);
	fx->process(buf);
	return samples;
}

void Effect::Output::reset()
{
	fx->resetState();
}

int Effect::Output::getSampleRate()
{
	return fx->sample_rate;
}

void Effect::Output::setSource(AudioSource *_source)
{
	source = _source;
	fx->sample_rate = source->getSampleRate();
}

Effect::Effect() :
	Configurable(TYPE_EFFECT)
{
	usable = true;
	plugin = NULL;
	enabled = true;
	song = NULL;
	sample_rate = DEFAULT_SAMPLE_RATE;
	out = new Output(this);
}

Effect::Effect(Plugin *p) :
	Configurable(TYPE_EFFECT)
{
	usable = true;
	plugin = p;
	enabled = true;
	song = NULL;
	sample_rate = DEFAULT_SAMPLE_RATE;
	out = new Output(this);
}

Effect::~Effect()
{
	delete out;
}

void Effect::__init__()
{
	new(this) Effect;
}

void Effect::__delete__()
{
	this->Effect::~Effect();
}

/*void Effect::prepare()
{
	resetState();
}*/

string Effect::getError()
{
	if (plugin)
		return plugin->getError();
	return format(_("Can't load effect: \"%s\""), name.c_str());
}



void Effect::doProcessTrack(Track *t, int layer, const Range &r)
{
	sample_rate = t->song->sample_rate;

	AudioBuffer buf = t->getBuffers(layer, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, layer, r);
	process(buf);
	song->execute(a);
}


Effect *CreateEffect(const string &name, Song *song)
{
	Plugin *p = tsunami->plugin_manager->GetPlugin(Plugin::TYPE_EFFECT, name);
	Effect *fx = NULL;
	if (p->usable)
		fx = (Effect*)p->createInstance("AudioEffect");

	// dummy?
	if (!fx)
		fx = new Effect;

	fx->name = name;
	fx->plugin = p;
	fx->usable = p->usable;
	fx->song = song;
	fx->resetConfig();
	return fx;
}
