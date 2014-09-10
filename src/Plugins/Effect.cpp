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
	Configurable("Effect", CONFIGURABLE_EFFECT)
{
	usable = true;
	plugin = NULL;
	only_on_selection = false;
	enabled = true;
}

Effect::Effect(Plugin *p) :
	Configurable("Effect", CONFIGURABLE_EFFECT)
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

void Effect::Prepare()
{
	msg_db_f("Effect.Prepare", 1);
	ResetState();
	if (!usable)
		tsunami->log->Error(GetError());
}

string Effect::GetError()
{
	if (plugin)
		return plugin->GetError();
	return format(_("Effekt nicht ladbar: \"%s\""), name.c_str());
}

void Effect::Apply(BufferBox &buf, Track *t, bool log_error)
{
	msg_db_f("Effect.Apply", 1);

	// run
	tsunami->plugin_manager->context.set(t, 0, buf.range());
	ProcessTrack(&buf);

	if (!usable){
		msg_error("not usable... apply");
		if (log_error)
			tsunami->log->Error(_("Beim Anwenden eines Effekts: ") + GetError());
	}
}



void Effect::DoProcessTrack(Track *t, int level_no, const Range &r)
{
	msg_db_f("Effect.DoProcessTrack", 1);

	tsunami->plugin_manager->context.set(t, level_no, r);

	BufferBox buf = t->GetBuffers(level_no, r);
	ActionTrackEditBuffer *a = new ActionTrackEditBuffer(t, level_no, r);
	ProcessTrack(&buf);
	t->root->Execute(a);
}


Effect *CreateEffect(const string &name)
{
	Effect *f = tsunami->plugin_manager->LoadEffect(name);
	if (f){
		f->name = name;
		f->ResetConfig();
		return f;
	}
	f = new Effect;
	f->name = name;
	f->plugin = tsunami->plugin_manager->GetPlugin(name);
	if (f->plugin){
		f->usable = f->plugin->usable;
	}
	return f;
}
