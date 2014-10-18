/*
 * Clipboard.cpp
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#include "Clipboard.h"
#include "../Data/AudioFile.h"
#include "../View/AudioView.h"
#include "../Action/Track/Sample/ActionTrackPasteAsSample.h"
#include <assert.h>

Clipboard::Clipboard() :
	Observable("Clipboard")
{
	type = -1;
	buf = NULL;
	midi = NULL;
	ref_uid = -1;
	sample_rate = DEFAULT_SAMPLE_RATE;
}

Clipboard::~Clipboard()
{
	Clear();
}

void Clipboard::Clear()
{
	if (buf){
		delete(buf);
		buf = NULL;
		Notify();
	}
	if (midi){
		delete(midi);
		midi = NULL;
		Notify();
	}
	type = -1;
	ref_uid = -1;
}

void Clipboard::Copy(AudioView *view)
{
	if (!CanCopy(view))
		return;
	Clear();

	AudioFile *a = view->audio;
	assert(a == view->cur_track->root);

	sample_rate = a->sample_rate;


	type = view->cur_track->type;
	if (view->cur_track->type == Track::TYPE_AUDIO){
		buf = new BufferBox;
		*buf = view->cur_track->ReadBuffers(view->cur_level, view->sel_range);
		buf->make_own();
	}else if (view->cur_track->type == Track::TYPE_MIDI){
		midi = new MidiData;
		midi->append(view->cur_track->midi.GetNotes(view->sel_range));
		foreach(MidiNote &n, *midi)
			n.range.offset -= view->sel_range.offset;
	}else
		type = -1;

	Notify();
}

void Clipboard::Paste(AudioView *view)
{
	if (!HasData())
		return;
	if (type != view->cur_track->type)
		return;
	AudioFile *a = view->audio;
	if (type == Track::TYPE_AUDIO){
		int index = a->get_sample_by_uid(ref_uid);
		if (index >= 0){
			view->cur_track->AddSample(view->sel_range.start(), index);
		}else{
			a->Execute(new ActionTrackPasteAsSample(view->cur_track, view->sel_range.start(), buf));
			ref_uid = a->sample.back()->uid;
		}
	}else if (type == Track::TYPE_MIDI){
		a->action_manager->BeginActionGroup();
		foreach(MidiNote &n, *midi){
			MidiNote nn = n;
			nn.range.offset += view->sel_range.start();
			view->cur_track->AddMidiNote(nn);
		}
		a->action_manager->EndActionGroup();
	}
}

bool Clipboard::HasData()
{
	return buf or midi;
}

bool Clipboard::CanCopy(AudioView *view)
{
	if (!view->cur_track)
		return false;
	if ((view->cur_track->type != Track::TYPE_AUDIO) and (view->cur_track->type != Track::TYPE_MIDI))
		return false;
	return !view->sel_range.empty();// or (view->audio->GetNumSelectedSamples() > 0);
}

