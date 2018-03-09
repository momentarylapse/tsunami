/*
 * Clipboard.cpp
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#include "Clipboard.h"
#include "../View/AudioView.h"
#include "../Action/Track/Sample/ActionTrackPasteAsSample.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "../Session.h"
#include "../Data/Song.h"
#include "../Data/SongSelection.h"
#include <assert.h>

Clipboard::Clipboard()
{
	temp = new Song(Session::GLOBAL);
	temp->reset();
}

Clipboard::~Clipboard()
{
	delete(temp);
}

void Clipboard::clear()
{
	if (temp->tracks.num > 0){
		temp->reset();
		notify();
	}
	ref_uid.clear();
}

void Clipboard::append_track(Track *t, AudioView *view)
{
	if (t->type == Track::TYPE_TIME)
		return;

	Track *tt = temp->addTrack(t->type);

	if (t->type == Track::TYPE_AUDIO){
		tt->layers[0].buffers.add(t->readBuffers(view->cur_layer, view->sel.range));
		tt->layers[0].buffers[0].make_own();
	}else if (t->type == Track::TYPE_MIDI){
		tt->midi = t->midi.getNotesBySelection(view->sel);
		tt->midi.samples = view->sel.range.length;
		tt->midi.sanify(view->sel.range);
		for (MidiNote *n: tt->midi)
			n->range.offset -= view->sel.range.offset;
	}

	ref_uid.add(-1);
}

void Clipboard::copy(AudioView *view)
{
	if (!canCopy(view))
		return;
	clear();

	Song *s = view->song;

	temp->sample_rate = s->sample_rate;

	for (Track *t: s->tracks)
		if (view->sel.has(t))
			append_track(t, view);

	notify();
}

void Clipboard::paste_track(int source_index, Track *target, AudioView *view)
{
	Song *s = target->song;
	Track *source = temp->tracks[source_index];

	if (target->type == Track::TYPE_AUDIO){
		Range r = Range(view->sel.range.start(), source->layers[0].buffers[0].length);
		AudioBuffer buf = target->getBuffers(view->cur_layer, r);
		Action *a = new ActionTrackEditBuffer(target, view->cur_layer, r);
		buf.set(source->layers[0].buffers[0], 0, 1.0f);
		s->execute(a);
	}else if (target->type == Track::TYPE_MIDI){
		for (MidiNote *n: source->midi){
			MidiNote nn = *n;
			nn.range.offset += view->sel.range.start();
			target->addMidiNote(nn);
		}
	}
}

void Clipboard::paste_track_as_samples(int source_index, Track *target, AudioView *view)
{
	Song *s = target->song;
	Track *source = temp->tracks[source_index];

	Sample* ref = s->get_sample_by_uid(ref_uid[source_index]);
	if (ref){
		target->addSampleRef(view->sel.range.start(), ref);
	}else{
		if (target->type == Track::TYPE_AUDIO){
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, view->sel.range.start(), source->layers[0].buffers[0], true));
			ref_uid[source_index] = sample->uid;
		}else if (target->type == Track::TYPE_MIDI){
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, view->sel.range.start(), source->midi, true));
			ref_uid[source_index] = sample->uid;
		}
	}
}

bool Clipboard::test_compatibility(AudioView *view)
{
	Array<string> temp_type, dest_type;
	for (Track *t: view->song->tracks){
		if (!view->sel.has(t))
			continue;
		if (t->type == Track::TYPE_TIME)
			continue;
		dest_type.add(track_type(t->type));
	}

	for (Track *t: temp->tracks)
		temp_type.add(track_type(t->type));

	if (dest_type.num != temp->tracks.num){
		view->session->e(format(_("%d tracks selected for pasting (ignoring the metronome), but %d tracks in clipboard"), dest_type.num, temp->tracks.num));
		return false;
	}
	string t1 = "[" + implode(temp_type, ", ") + "]";
	string t2 = "[" + implode(dest_type, ", ") + "]";
	if (t1 != t2){
		view->session->e(format(_("Track types in clipboard (%s) don't match those you want to paste into (%s)"), t1.c_str(), t2.c_str()));
		return false;
	}
	return true;
}

void Clipboard::paste(AudioView *view)
{
	if (!hasData())
		return;
	Song *s = view->song;

	if (!test_compatibility(view))
		return;


	s->action_manager->beginActionGroup();

	int ti = 0;
	for (Track *t: s->tracks){
		if (!view->sel.has(t))
			continue;
		if (t->type == Track::TYPE_TIME)
			continue;

		paste_track(ti, t, view);
		ti ++;
	}
	s->action_manager->endActionGroup();
}

void Clipboard::pasteAsSamples(AudioView *view)
{
	if (!hasData())
		return;
	Song *s = view->song;

	if (!test_compatibility(view))
		return;


	s->action_manager->beginActionGroup();

	int ti = 0;
	for (Track *t: s->tracks){
		if (!view->sel.has(t))
			continue;
		if (t->type == Track::TYPE_TIME)
			continue;

		paste_track_as_samples(ti, t, view);
		ti ++;
	}
	s->action_manager->endActionGroup();
}

bool Clipboard::hasData()
{
	return (temp->tracks.num > 0);
}

bool Clipboard::canCopy(AudioView *view)
{
	return !view->sel.range.empty();// or (view->audio->GetNumSelectedSamples() > 0);
}

