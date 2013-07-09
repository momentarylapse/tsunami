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
#include "../Action/SubTrack/ActionSubTrackPaste.h"
#include <assert.h>

Clipboard::Clipboard() :
	Observable("Clipboard")
{
	buf = NULL;
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
}

void Clipboard::Copy(AudioFile *a)
{
	Clear();

	if (!a->selection.empty()){
		assert(a == tsunami->view->cur_track->root);

		sample_rate = a->sample_rate;

		buf = new BufferBox;
		*buf = tsunami->view->cur_track->ReadBuffers(tsunami->view->cur_level, a->selection);
		buf->make_own();

		Notify("Change");
	}
}

void Clipboard::Paste(AudioFile *a)
{
	if (!HasData())
		return;
	if (a->used){
		a->Execute(new ActionSubTrackPaste(get_track_index(tsunami->view->cur_track), a->selection.start(), buf));
	}else{
		a->NewWithOneTrack(sample_rate, Track::TYPE_AUDIO);
		a->action_manager->Enable(false);
		BufferBox dest = a->track[0]->GetBuffers(0, Range(0, buf->num));
		dest.set(*buf, 0, 1.0f);
		a->InvalidateAllPeaks();
		a->UpdatePeaks(tsunami->view->PeakMode);
		a->action_manager->Enable(true);
	}
}

bool Clipboard::HasData()
{
	return buf;
}

