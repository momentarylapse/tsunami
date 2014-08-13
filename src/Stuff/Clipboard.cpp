/*
 * Clipboard.cpp
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#include "Clipboard.h"
#include "../Data/AudioFile.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../View/AudioView.h"
#include "../Action/Track/Sample/ActionTrackPasteAsSample.h"
#include <assert.h>

Clipboard::Clipboard() :
	Observable("Clipboard")
{
	buf = NULL;
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

void Clipboard::Copy(AudioFile *a)
{
	if (!CanCopy(a))
		return;
	Clear();

	assert(a == tsunami->win->view->cur_track->root);

	sample_rate = a->sample_rate;

	buf = new BufferBox;
	*buf = tsunami->win->view->cur_track->ReadBuffers(tsunami->win->view->cur_level, tsunami->win->view->sel_range);
	buf->make_own();

	Notify();
}

void Clipboard::Paste(AudioFile *a)
{
	if (!HasData())
		return;
	if (a->used){
		int index = a->get_sample_by_uid(ref_uid);
		if (index >= 0){
			tsunami->win->view->cur_track->AddSample(tsunami->win->view->sel_range.start(), index);
		}else{
			a->Execute(new ActionTrackPasteAsSample(tsunami->win->view->cur_track, tsunami->win->view->sel_range.start(), buf));
			ref_uid = a->sample.back()->uid;
		}
	}else{
		a->NewWithOneTrack(sample_rate, Track::TYPE_AUDIO);
		a->action_manager->Enable(false);
		BufferBox dest = a->track[0]->GetBuffers(0, Range(0, buf->num));
		dest.set(*buf, 0, 1.0f);
		a->InvalidateAllPeaks();
		a->UpdatePeaks(tsunami->win->view->peak_mode);
		a->action_manager->Enable(true);
	}
}

bool Clipboard::HasData()
{
	return buf;
}

bool Clipboard::CanCopy(AudioFile *a)
{
	return !tsunami->win->view->sel_range.empty();// || (a->GetNumSelectedSamples() > 0);
}

