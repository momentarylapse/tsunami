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
	this->MidiEffect::~MidiEffect();
}

void MidiEffect::Prepare()
{
	resetState();
	if (!usable)
		tsunami->log->error(GetError());
}

string MidiEffect::GetError()
{
	if (plugin)
		return plugin->getError();
	return format(_("Can not load MidiEffect: \"%s\""), name.c_str());
}

void MidiEffect::Apply(MidiData &midi, Track *t, bool log_error)
{
	// run
	process(&midi);

	if (!usable){
		msg_error("not usable... apply");
		if (log_error)
			tsunami->log->error(_("While applying a midi effect: ") + GetError());
	}
}



void MidiEffect::DoProcessTrack(Track *t, const Range &r)
{
	MidiDataRef midi_ref = t->midi.getNotes(r);
	MidiData midi = t->midi.getNotes(r);

	t->song->action_manager->beginActionGroup();

	for (int i=t->midi.num-1; i>=0; i--)
		if (r.is_inside(t->midi[i]->range.center()))
			t->deleteMidiNote(i);

	process(&midi);
	for (MidiNote *n: midi)
		n->reset_meta();

	t->insertMidiData(0, midi);
	t->song->action_manager->endActionGroup();
}


MidiEffect *CreateMidiEffect(const string &name, Song *song)
{
	Plugin *p = tsunami->plugin_manager->GetPlugin(Plugin::TYPE_MIDI_EFFECT, name);
	MidiEffect *fx = NULL;
	if (p->usable)
		fx = (MidiEffect*)p->createInstance("MidiEffect");

	// dummy?
	if (!fx)
		fx = new MidiEffect;

	fx->name = name;
	fx->plugin = p;
	fx->usable = p->usable;
	fx->song = song;
	fx->resetConfig();
	return fx;
}

