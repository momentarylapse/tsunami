/*
 * MidiEffect.cpp
 *
 *  Created on: 10.09.2014
 *      Author: michi
 */

#include "MidiEffect.h"

#include "Plugin.h"
#include "../Session.h"
#include "../lib/math/math.h"
#include "PluginManager.h"
#include "../Data/SongSelection.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"

MidiEffect::MidiEffect() :
	Module(Session::GLOBAL, Type::MIDI_EFFECT)
{
	usable = true;
	plugin = NULL;
	only_on_selection = false;
	enabled = true;
	bh_offset = 0;
	bh_song = NULL;
	bh_midi = NULL;
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

void MidiEffect::prepare()
{
	reset_state();
	if (!usable)
		session->e(get_error());
}

void MidiEffect::apply(MidiNoteBuffer &midi, Track *t, bool log_error)
{
	// run
	process(&midi);

	if (!usable){
		msg_error("not usable... apply");
		if (log_error)
			session->e(_("While applying a midi effect: ") + get_error());
	}
}



void MidiEffect::process_track(Track *t, const SongSelection &sel)
{
	MidiNoteBuffer midi = t->midi.getNotesBySelection(sel);

	bh_song = t->song;
	bh_offset = sel.range.offset;
	int b2 = bh_song->bars.getNextBeat(bh_offset);
	int b1 = bh_song->bars.getPrevBeat(b2);
	if (b1 < bh_offset)
		bh_offset = b2;
	bh_midi = &midi;

	t->song->action_manager->beginActionGroup();

	for (int i=t->midi.num-1; i>=0; i--)
		if (sel.has(t->midi[i]))
			t->deleteMidiNote(i);

	process(&midi);
	for (MidiNote *n: midi)
		n->reset_meta();

	t->insertMidiData(0, midi);
	t->song->action_manager->endActionGroup();
}

void MidiEffect::note(float pitch, float volume, int beats)
{
	if (!bh_song)
		return;
	int pos = bh_offset;
	for (int i=0; i<beats; i++)
		pos = bh_song->bars.getNextBeat(pos);

	bh_midi->add(new MidiNote(Range(bh_offset, pos - bh_offset), pitch, volume));
	bh_offset = pos;

}

void MidiEffect::skip(int beats)
{
	if (!bh_song)
		return;

	for (int i=0; i<beats; i++)
		bh_offset = bh_song->bars.getNextBeat(bh_offset);
}

void MidiEffect::note_x(float pitch, float volume, int beats, int sub_beats, int beat_partition)
{
	if (!bh_song)
		return;
	int pos = bh_offset;
	for (int i=0; i<beats; i++)
		pos = bh_song->bars.getNextBeat(pos);
	for (int i=0; i<sub_beats; i++)
		pos = bh_song->bars.getNextSubBeat(pos, beat_partition);

	bh_midi->add(new MidiNote(Range(bh_offset, pos - bh_offset), pitch, volume));
	bh_offset = pos;

}

void MidiEffect::skip_x(int beats, int sub_beats, int beat_partition)
{
	if (!bh_song)
		return;

	for (int i=0; i<beats; i++)
		bh_offset = bh_song->bars.getNextBeat(bh_offset);
	for (int i=0; i<sub_beats; i++)
		bh_offset = bh_song->bars.getNextSubBeat(bh_offset, beat_partition);
}


MidiEffect *CreateMidiEffect(Session *session, const string &name)
{
	Plugin *p = session->plugin_manager->GetPlugin(session, Plugin::Type::MIDI_EFFECT, name);
	MidiEffect *fx = NULL;
	if (p->usable)
		fx = (MidiEffect*)p->create_instance(session, "MidiEffect");

	// dummy?
	if (!fx)
		fx = new MidiEffect;

	fx->name = name;
	fx->plugin = p;
	fx->usable = p->usable;
	fx->session = session;
	fx->reset_config();
	return fx;
}

