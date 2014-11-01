/*
 * MidiEffect.cpp
 *
 *  Created on: 10.09.2014
 *      Author: michi
 */

#include "MidiEffect.h"

#include "Plugin.h"
#include "../Tsunami.h"
#include "../lib/script/script.h"
#include "../lib/math/math.h"
#include "../Stuff/Log.h"
#include "PluginManager.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"

MidiEffect::MidiEffect() :
	Configurable("Effect", TYPE_MIDI_EFFECT)
{
	usable = true;
	plugin = NULL;
	only_on_selection = false;
	enabled = true;
}

MidiEffect::MidiEffect(Plugin *p) :
	Configurable("Effect", TYPE_MIDI_EFFECT)
{
	usable = true;
	plugin = p;
	only_on_selection = false;
	enabled = true;
}

MidiEffect::~MidiEffect()
{
}

void MidiEffect::__init__()
{
	new(this) MidiEffect;
}

void MidiEffect::__delete__()
{
	this->Configurable::~Configurable();
}

void MidiEffect::Prepare()
{
	msg_db_f("MidiEffect.Prepare", 1);
	resetState();
	if (!usable)
		tsunami->log->error(GetError());
}

string MidiEffect::GetError()
{
	if (plugin)
		return plugin->GetError();
	return format(_("MidiEffekt nicht ladbar: \"%s\""), name.c_str());
}

void MidiEffect::Apply(MidiData &midi, Track *t, bool log_error)
{
	msg_db_f("MidiEffect.Apply", 1);

	// run
	tsunami->plugin_manager->context.set(t, 0, t->GetRange());
	process(&midi);

	if (!usable){
		msg_error("not usable... apply");
		if (log_error)
			tsunami->log->error(_("Beim Anwenden eines MidiEffekts: ") + GetError());
	}
}



void MidiEffect::DoProcessTrack(Track *t, const Range &r)
{
	msg_db_f("MidiEffect.DoProcessTrack", 1);

	tsunami->plugin_manager->context.set(t, 0, r);

	MidiData midi;
	midi.append(t->midi.GetNotes(r));

	t->root->action_manager->BeginActionGroup();

	foreachib(MidiNote &n, t->midi, i)
		if (r.is_inside(n.range.offset)){
			t->DeleteMidiNote(i);
			_foreach_it_.update(); // TODO...
		}
	process(&midi);
	t->InsertMidiData(0, midi);
	t->root->action_manager->EndActionGroup();
}


MidiEffect *CreateMidiEffect(const string &name)
{
	MidiEffect *f = tsunami->plugin_manager->LoadMidiEffect(name);
	if (f){
		f->name = name;
		f->resetConfig();
		return f;
	}
	f = new MidiEffect;
	f->name = name;
	f->plugin = tsunami->plugin_manager->GetPlugin(name);
	if (f->plugin){
		f->usable = f->plugin->usable;
	}
	return f;
}

