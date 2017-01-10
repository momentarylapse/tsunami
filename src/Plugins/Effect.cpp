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

Effect::Effect() :
	Configurable("Effect", TYPE_EFFECT)
{
	usable = true;
	plugin = NULL;
	only_on_selection = false;
	enabled = true;
	song = NULL;
	track = NULL;
	level = 0;
}

Effect::Effect(Plugin *p) :
	Configurable("Effect", TYPE_EFFECT)
{
	usable = true;
	plugin = p;
	only_on_selection = false;
	enabled = true;
	song = NULL;
	track = NULL;
	level = 0;
}

Effect::~Effect()
{
}

void Effect::__init__()
{
	new(this) Effect;
}

void Effect::__delete__()
{
	this->Effect::~Effect();
}

void Effect::prepare()
{
	resetState();
}

string Effect::getError()
{
	if (plugin)
		return plugin->getError();
	return format(_("Can't load effect: \"%s\""), name.c_str());
}

void Effect::apply(BufferBox &buf, Track *t, bool log_error)
{
	track = t;
	song = t->song;
	level = 0;
	range = buf.range();

	// run
	processTrack(&buf);
}



void Effect::doProcessTrack(Track *t, int _level, const Range &r)
{
	track = t;
	song = t->song;
	level = _level;
	range = r;

	BufferBox buf = t->getBuffers(level, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(track, level, r);
	processTrack(&buf);
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
