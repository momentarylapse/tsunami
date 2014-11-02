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
#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include <assert.h>

Clipboard::Clipboard() :
	Observable("Clipboard")
{
	temp = new AudioFile;
	temp->reset();
}

Clipboard::~Clipboard()
{
	delete(temp);
}

void Clipboard::clear()
{
	if (temp->track.num > 0){
		temp->reset();
		notify();
	}
	ref_uid.clear();
}

void Clipboard::copy(AudioView *view)
{
	if (!canCopy(view))
		return;
	clear();

	AudioFile *a = view->audio;

	temp->sample_rate = a->sample_rate;

	foreach(Track *t, a->track){
		if (!t->is_selected)
			continue;
		if (t->type == Track::TYPE_TIME)
			continue;
		Track *tt = temp->addTrack(t->type);

		if (t->type == Track::TYPE_AUDIO){
			tt->level[0].buffer.add(t->readBuffers(view->cur_level, view->sel_range));
			tt->level[0].buffer[0].make_own();
		}else if (t->type == Track::TYPE_MIDI){
			tt->midi.append(t->midi.getNotes(view->sel_range));
			foreach(MidiNote &n, tt->midi)
				n.range.offset -= view->sel_range.offset;
		}
		ref_uid.add(-1);
	}

	notify();
}

void Clipboard::paste(AudioView *view)
{
	if (!hasData())
		return;
	AudioFile *a = view->audio;

	Array<string> temp_type, dest_type;
	foreach(Track *t, a->track){
		if (!t->is_selected)
			continue;
		if (t->type == Track::TYPE_TIME)
			continue;
		dest_type.add(track_type(t->type));
	}
	foreach(Track *t, temp->track)
		temp_type.add(track_type(t->type));
	if (dest_type.num != temp->track.num){
		tsunami->log->error(format(_("%d Spuren zum Einf&ugen markiert (ohne Metronom gez&ahlt), aber %d Spuren in der Zwischenablage"), dest_type.num, temp->track.num));
		return;
	}
	string t1 = "[" + implode(temp_type, ", ") + "]";
	string t2 = "[" + implode(dest_type, ", ") + "]";
	if (t1 != t2){
		tsunami->log->error(format(_("Spurtypen in der Zwischenablage (%s) passen nicht zu den Spuren, in die eingef&ugt werden soll (%s)"), t1.c_str(), t2.c_str()));
		return;
	}


	a->action_manager->beginActionGroup();
	int ti = 0;
	foreach(Track *t, a->track){
		if (!t->is_selected)
			continue;
		if (t->type == Track::TYPE_TIME)
			continue;

		Track *tt = temp->track[ti];
		int ref_index = a->get_sample_by_uid(ref_uid[ti]);
		if (ref_index >= 0){
			t->addSample(view->sel_range.start(), ref_index);
		}else{
			if (t->type == Track::TYPE_AUDIO){
				a->execute(new ActionTrackPasteAsSample(t, view->sel_range.start(), &tt->level[0].buffer[0]));
				ref_uid[ti] = a->sample.back()->uid;
			}else if (t->type == Track::TYPE_MIDI){
				a->execute(new ActionTrackPasteAsSample(t, view->sel_range.start(), &tt->midi));
				ref_uid[ti] = a->sample.back()->uid;
			}
		}

		ti ++;
	}
	a->action_manager->endActionGroup();
}

bool Clipboard::hasData()
{
	return (temp->track.num > 0);
}

bool Clipboard::canCopy(AudioView *view)
{
	return !view->sel_range.empty();// or (view->audio->GetNumSelectedSamples() > 0);
}

