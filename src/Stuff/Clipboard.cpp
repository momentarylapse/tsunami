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

	buf = new BufferBox;
	*buf = view->cur_track->ReadBuffers(view->cur_level, view->sel_range);
	buf->make_own();

	Notify();
}

void Clipboard::Paste(AudioView *view)
{
	if (!HasData())
		return;
	AudioFile *a = view->audio;
	int index = a->get_sample_by_uid(ref_uid);
	if (index >= 0){
		view->cur_track->AddSample(view->sel_range.start(), index);
	}else{
		a->Execute(new ActionTrackPasteAsSample(view->cur_track, view->sel_range.start(), buf));
		ref_uid = a->sample.back()->uid;
	}
}

bool Clipboard::HasData()
{
	return buf;
}

bool Clipboard::CanCopy(AudioView *view)
{
	return !view->sel_range.empty();// || (view->audio->GetNumSelectedSamples() > 0);
}

