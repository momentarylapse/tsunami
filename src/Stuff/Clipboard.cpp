/*
 * Clipboard.cpp
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#include "Clipboard.h"
#include "../Data/AudioFile.h"
#include "../Tsunami.h"
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
		Notify("Change");
	}
	ref_uid = -1;
}

void Clipboard::Copy(AudioFile *a)
{
	if (!CanCopy(a))
		return;
	Clear();

	assert(a == tsunami->view->cur_track->root);

	sample_rate = a->sample_rate;

	buf = new BufferBox;
	*buf = tsunami->view->cur_track->ReadBuffers(tsunami->view->cur_level, a->selection);
	buf->make_own();

	Notify("Change");
}

void Clipboard::Paste(AudioFile *a)
{
	if (!HasData())
		return;
	if (a->used){
		int index = a->get_sample_by_uid(ref_uid);
		if (index >= 0){
			tsunami->view->cur_track->AddSample(a->selection.start(), index);
		}else{
			a->Execute(new ActionTrackPasteAsSample(tsunami->view->cur_track, a->selection.start(), buf));
			ref_uid = a->sample.back()->uid;
		}
	}else{
		a->NewWithOneTrack(sample_rate, Track::TYPE_AUDIO);
		a->action_manager->Enable(false);
		BufferBox dest = a->track[0]->GetBuffers(0, Range(0, buf->num));
		dest.set(*buf, 0, 1.0f);
		a->InvalidateAllPeaks();
		a->UpdatePeaks(tsunami->view->peak_mode);
		a->action_manager->Enable(true);
	}
}

bool Clipboard::HasData()
{
	return buf;
}

bool Clipboard::CanCopy(AudioFile *a)
{
	return !a->selection.empty();// || (a->GetNumSelectedSamples() > 0);
}

