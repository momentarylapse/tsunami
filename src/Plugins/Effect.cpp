/*
 * Effect.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Effect.h"

#include "Plugin.h"
#include "../Session.h"
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
		return buf.length;
	int samples = source->read(buf);
	fx->process(buf);
	return samples;
}

void Effect::Output::reset()
{
	fx->resetState();
	if (source)
		source->reset();
}

int Effect::Output::get_pos(int delta)
{
	if (!source)
		return -1;
	return source->get_pos(delta);
}

int Effect::Output::sample_rate()
{
	return fx->sample_rate;
}

void Effect::Output::set_source(AudioSource *_source)
{
	source = _source;
	if (source)
		fx->sample_rate = source->sample_rate();
}

Effect::Effect() :
	Configurable(Session::GLOBAL, Type::EFFECT)
{
	usable = true;
	plugin = NULL;
	enabled = true;
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
	session->song->execute(a);
}


// TODO: move to PluginManager?
Effect *CreateEffect(Session *session, const string &name)
{
	Plugin *p = session->plugin_manager->GetPlugin(session, Plugin::Type::EFFECT, name);
	Effect *fx = NULL;
	if (p->usable)
		fx = (Effect*)p->createInstance(session, "AudioEffect");

	// dummy?
	if (!fx)
		fx = new Effect;

	fx->name = name;
	fx->plugin = p;
	fx->usable = p->usable;
	fx->song = session->song;
	fx->session = session;
	fx->resetConfig();
	return fx;
}
