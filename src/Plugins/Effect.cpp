/*
 * Effect.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Effect.h"

#include "Plugin.h"
#include "../Tsunami.h"
#include "../lib/script/script.h"
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
}

Effect::Effect(Plugin *p) :
	Configurable("Effect", TYPE_EFFECT)
{
	usable = true;
	plugin = p;
	only_on_selection = false;
	enabled = true;
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
	this->Configurable::~Configurable();
}

void Effect::prepare()
{
	msg_db_f("Effect.Prepare", 1);
	resetState();
	if (!usable)
		tsunami->log->error(getError());
}

string Effect::getError()
{
	if (plugin)
		return plugin->GetError();
	return format(_("Can't load effect: \"%s\""), name.c_str());
}

void Effect::apply(BufferBox &buf, Track *t, bool log_error)
{
	msg_db_f("Effect.Apply", 1);

	// run
	tsunami->plugin_manager->context.set(t, 0, buf.range());
	processTrack(&buf);

	if (!usable){
		msg_error("not usable... apply");
		if (log_error)
			tsunami->log->error(_("While applying an effect: ") + getError());
	}
}



void Effect::doProcessTrack(Track *t, int level_no, const Range &r)
{
	msg_db_f("Effect.DoProcessTrack", 1);

	tsunami->plugin_manager->context.set(t, level_no, r);

	BufferBox buf = t->getBuffers(level_no, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, level_no, r);
	processTrack(&buf);
	t->song->execute(a);
}


Effect *CreateEffect(const string &name, Song *song)
{
	Effect *f = tsunami->plugin_manager->LoadEffect(name, song);
	if (f){
		f->name = name;
		f->resetConfig();
		return f;
	}
	f = new Effect;
	f->song = song;
	f->name = name;
	f->plugin = tsunami->plugin_manager->GetPlugin(name);
	if (f->plugin){
		f->usable = f->plugin->usable;
	}
	return f;
}
